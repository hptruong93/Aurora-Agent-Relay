#include <thread>
#include <iostream>

#include "mon_to_warp_agent.h"
#include "wlan_to_warp_agent.h"
#include "warp_to_wlan_agent.h"
#include "relay_agent.h"
#include "comms_agent.h"
#include "util.h"
// Testing
#include "test.h"

using namespace RelayAgents;
using namespace std;

uint8_t parse_mac(const char* origin, uint8_t dest[])
{
    if (sscanf(origin, "%02hhx:%02hhx:%02hhx:%02hhx:%02hhx:%02hhx", &dest[0], &dest[1], &dest[2], &dest[3], &dest[4], &dest[5]) != 6)
    {
        cout<<"ERROR: Failed to parse mac address string into uint arrays"<<endl;

        return RETURN_CODE_ERROR;
    }

    return RETURN_CODE_OK;
}

uint8_t parse_hwmode(std::string& hwmode)
{
    uint8_t a = hwmode.find('a') == string::npos ? 0x00 : 0x08;
    uint8_t b = hwmode.find('b') == string::npos ? 0x00 : 0x04;
    uint8_t g = hwmode.find('g') == string::npos ? 0x00 : 0x02;
    uint8_t n = hwmode.find('n') == string::npos ? 0x00 : 0x01;

    return a | b | g | n;
}

CommsAgent::CommsAgent(const char *init_send_port, const char *init_recv_port, const char *init_peer_ip_addr)
{
    send_port = unique_ptr<string>(new string(init_send_port));
    recv_port = unique_ptr<string>(new string(init_recv_port));
    peer_ip_addr = unique_ptr<string>(new string(init_peer_ip_addr));

    // Consume the semaphore
    sem_init(&this->signal, 0, 1);
    sem_wait(&this->signal);

    // Consume new command semaphore
    sem_init(&this->new_command, 0, 1);
    sem_wait(&this->new_command);
}

void CommsAgent::send_loop()
{
    zmq::context_t ctx(1);
    zmq::socket_t pub_socket = zmq::socket_t(ctx, ZMQ_PUB);

    std::cout << "Setting up send loop" << string("tcp://*:") + *this->send_port.get() << std::endl;
    string pub_address = string("tcp://*:") + *this->send_port.get();
    pub_socket.bind(pub_address.c_str());

    while (true)
    {
        sem_wait(&this->signal);
        cout << "Send Loop: Sending response... " << this->send_message.get()->c_str() << endl;
        this->message_lock.lock();

        std::string to_send = *this->send_message.get();
        zmq::message_t send_message(to_send.length() + 1);

        snprintf((char*)send_message.data(), to_send.length() + 1, to_send.c_str());
        pub_socket.send(send_message);

        this->message_lock.unlock();
    }

    cout << "Exiting send loop ?" << endl;
}

void CommsAgent::recv_loop()
{
    zmq::message_t received_msg;

    zmq::context_t ctx(1);
    zmq::socket_t sub_socket = zmq::socket_t(ctx, ZMQ_SUB);

    string sub_address = string("tcp://") + *this->peer_ip_addr.get() + string(":") + *this->recv_port.get();
    sub_socket.connect(sub_address.c_str());

    // Set socket options for receive socket
    // Receive everything for now
    sub_socket.setsockopt(ZMQ_SUBSCRIBE, "", 0);

    while(true)
    {
        sub_socket.recv(&received_msg);
        // Add to command queue
        this->command_queue_lock.lock();
        this->command_queue.push(string((char*)(received_msg.data() + 4)));
        // cout<<"PUSHED!!: "<<string((char*)(received_msg.data() + 4))<<endl;
        // cout<<"SIZE AFTER PUSH: "<<this->command_queue.size()<<endl;
        this->command_queue_lock.unlock();

        sem_post(&this->new_command);
    }
}

void CommsAgent::parse_loop()
{
    while (true)
    {
        sem_wait(&this->new_command);

        while(this->command_queue.size() > 0)
        {
            this->command_queue_lock.lock();
            string next_command = this->command_queue.front();
            // cout<<"PEEK: "<<this->command_queue.front()<<endl;
            // cout<<"SIZE WHEN PEEK: "<<this->command_queue.size()<<endl;
            this->command_queue_lock.unlock();

            uint8_t code = this->parse_json(next_command.c_str());
            printf("Should we response ? %d\n", code);

            if (code & RETURN_CODE_SEND_RESPONSE)
            {
                printf("Yea response ...\n");
                sem_post(&this->signal);
            }

            this->command_queue_lock.lock();
            this->command_queue.pop();
            // cout<<"POPPED SOMETHING!"<<endl;
            // cout<<"SIZE AFTER POP: "<<this->command_queue.size()<<endl;
            this->command_queue_lock.unlock();
        }
    }
}

uint8_t CommsAgent::parse_json(const char *json_string)
{
    uint8_t if_send_response = RETURN_CODE_NO_RESPONSE;
    json_t *root;
    json_error_t error;

    std::cout << "Parsing " << json_string << endl;
    root = json_loads(json_string, 0, &error);

    if (!root)
    {
        if (strcmp(json_string, "test") == 0)
        {
            // Test string
            this->set_msg("{ \"changes\": { \"success\": True, \"message\": \"Test.\"} }");
            return RETURN_CODE_OK | RETURN_CODE_SEND_RESPONSE;
        }

        cout<<"ERROR: on line "<<error.line<<": "<<error.text<<endl;

        return RETURN_CODE_ERROR | RETURN_CODE_NO_RESPONSE;
    }

    // Test if root is a valid json object
    if (!json_is_object(root))
    {
        cout << "ERROR: root is not a json object." <<endl;
        json_decref(root);

        return RETURN_CODE_ERROR | RETURN_CODE_NO_RESPONSE;
    }

    // Get command type
    json_t *command = json_object_get(root, JSON_COMMAND);
    if (!json_is_string(command))
    {
        cout << "ERROR: command is not a string." <<endl;

        json_decref(root);

        return RETURN_CODE_ERROR | RETURN_CODE_NO_RESPONSE;
    }

    // Used for response messages
    uint8_t response;

    // Parse command
    const char *command_str = json_string_value(command);
    std::string current_command(command_str);
    std::string current_radio;

    if (strcmp(command_str, RADIO_SET_CMD) == 0 || 
        strcmp(command_str, RADIO_BULK_SET_CMD) == 0)
    {
        if_send_response = RETURN_CODE_SEND_RESPONSE;
        // Get changes object
        json_t *changes = json_object_get(root, JSON_CHANGES);
        if (!json_is_object(changes))
        {
            cout << "ERROR: no valid changes object found." <<endl;
            this->set_error_msg(current_command, "\"changes\" is not a valid json object");

            json_decref(root);

            return RETURN_CODE_ERROR | RETURN_CODE_SEND_RESPONSE;
        }

        json_t *radio;
        radio = json_object_get(root, JSON_RADIO);
        if (!json_is_string(radio) )
        {
            cout << "ERROR: radio is not a valid string." <<endl;
            this->set_error_msg(current_command, "radio is not a valid string");

            json_decref(root);

            return RETURN_CODE_ERROR | RETURN_CODE_SEND_RESPONSE;
        }

        current_radio = string(json_string_value(radio));

        // Parse the changes
        json_t *parameters, *bssid;
        bssid = json_object_get(changes, JSON_MAC_ADDRESS);
        if (!json_is_string(bssid) )
        {
            cout << "ERROR: bssid is not a valid string." <<endl;
            this->set_error_msg(current_command, "macaddr is not a valid string");

            json_decref(root);

            return RETURN_CODE_ERROR | RETURN_CODE_SEND_RESPONSE;
        }

        // Bssid parsing and send mac address control protocol
        WARP_protocol::WARP_mac_control_struct mac_address_cntrl_struct;
        mac_address_cntrl_struct.operation_code = MAC_ADD_CODE;
        if (parse_mac(json_string_value(bssid), mac_address_cntrl_struct.mac_address) != RETURN_CODE_OK)
        {
            json_decref(root);

            cout << "ERROR: invalid bssid format." << endl;
            this->set_error_msg(current_command, "Invalid mac address in json object.");
            return RETURN_CODE_ERROR | RETURN_CODE_SEND_RESPONSE;
        }

        cout << "before sending to WARP" << endl;

        // Parsing done. We can send the mac address control now
        WARP_protocol *mac_add_packet = WARP_protocol::create_mac_control(&mac_address_cntrl_struct);
        this->warp_to_wlan_agent.get()->sync(BSSID_NODE_OPS::SEND_MAC_ADDR_CNTRL, mac_add_packet);

        // Wait until WARP talks back
        if (this->warp_to_wlan_agent.get()->timed_sync((int)BSSID_NODE_OPS::MAC_ADD, &response, 500) == -1
            || response != MAC_ADD_CODE)
        {
            delete mac_add_packet;

            json_decref(root);

            cout << "ERROR: WARP failed to add mac address, or the request timed out." << endl;
            this->set_error_msg(current_command, "WARP failed to add mac address, or the request timed out.");

            cout << "after sending to WARP" << endl;
            return RETURN_CODE_ERROR | RETURN_CODE_SEND_RESPONSE;
        }

        // Mac address successfully added by WARP
        delete mac_add_packet;

        // Start constructing transmission control
        WARP_protocol::WARP_transmission_control_struct *transmission_control = WARP_protocol::get_default_transmission_control_struct();
        // We know by this point Bssid is valid both in value and format, so no error detection here
        parse_mac(json_string_value(bssid), transmission_control->bssid);

        // Parsing the changes parameters
        parameters = json_object_get(changes, JSON_CHANNEL);
        if (!json_is_integer(parameters))
        {
            cout << "ERROR: invalid channel format." << endl;
            this->set_error_msg(current_command, "Invalid channel format.");

            free(transmission_control);
            json_decref(root);

            return RETURN_CODE_ERROR | RETURN_CODE_SEND_RESPONSE;
        }
        transmission_control->channel = (uint8_t)json_integer_value(parameters);

        parameters = json_object_get(changes, JSON_HW_MODE);
        if (!json_is_string(parameters))
        {
            cout << "ERROR: invalid hw_mode format." << endl;
            this->set_error_msg(current_command, "Invalid hw_mode format");

            free(transmission_control);
            json_decref(root);

            return RETURN_CODE_ERROR | RETURN_CODE_SEND_RESPONSE;
        }
        string hwmode_str(json_string_value(parameters));
        transmission_control->hw_mode = parse_hwmode(hwmode_str);

        parameters = json_object_get(changes, JSON_TX_POWER);
        if (!json_is_integer(parameters))
        {
            cout << "ERROR: invalid tx_power format." << endl;
            this->set_error_msg(current_command, "Invalid tx_power format.");

            free(transmission_control);
            json_decref(root);

            return RETURN_CODE_ERROR | RETURN_CODE_SEND_RESPONSE;
        }
        transmission_control->tx_power = (uint8_t)json_integer_value(parameters);
        parameters = json_object_get(changes, JSON_DISABLED);
        if (!json_is_integer(parameters))
        {
            cout << "ERROR: invalid disabled format." << endl;
            this->set_error_msg(current_command, "Invalid disabled format.");

            free(transmission_control);
            json_decref(root);

            return RETURN_CODE_ERROR | RETURN_CODE_SEND_RESPONSE;
        }
        transmission_control->disabled = (uint8_t)json_integer_value(parameters);

        // Done! Time to send
        WARP_protocol *transmission_packet = WARP_protocol::create_transmission_control(transmission_control);
        this->warp_to_wlan_agent.get()->sync(BSSID_NODE_OPS::SEND_TRANSMISSION_CNTRL, transmission_packet);

        if (this->warp_to_wlan_agent.get()->timed_sync((int)BSSID_NODE_OPS::TRANSMISSION_CNTRL, &response, 500) == -1
            || response != TRANSMISSION_CONFIGURE_SUCCESS_CODE)
        {
            delete transmission_packet;
            free(transmission_control);
            json_decref(root);

            cout << "ERROR: WARP failed to set up the configuration requested, or the request timed out." << endl;
            this->set_error_msg(current_command, "WARP failed to set up the configuration requested, or the request timed out.");
            return RETURN_CODE_ERROR | RETURN_CODE_SEND_RESPONSE;
        }

        // Configuration successful!!
        delete transmission_packet;
        free(transmission_control);
        
        // Update corresponding bssid nodes
        this->update_bssids(BSSID_NODE_OPS::BSSID_ADD, (void*)json_string_value(bssid));
    }
    else if (strcmp(command_str, UCI_DELETE_SECTION) == 0 ||
                strcmp(command_str, UCI_DELETE_BSS) == 0)
    {
        if_send_response = RETURN_CODE_SEND_RESPONSE;
        json_t *changes = json_object_get(root, JSON_CHANGES);

        json_t *radio;
        radio = json_object_get(root, JSON_RADIO);
        if (!json_is_string(radio) )
        {
            cout << "ERROR: radio is not a valid string." <<endl;
            this->set_error_msg(current_command, "radio is not a valid string");

            json_decref(root);

            return RETURN_CODE_ERROR | RETURN_CODE_SEND_RESPONSE;
        }

        current_radio = string(json_string_value(radio));

        // Get Bssid 
        json_t *bssid;
        bssid = json_object_get(changes, JSON_MAC_ADDRESS);
        if (!json_is_string(bssid))
        {
            cout << "ERROR: bssid is not a valid string." <<endl;
            this->set_error_msg(current_command, "macaddr is not a string.");

            json_decref(root);

            return RETURN_CODE_ERROR | RETURN_CODE_SEND_RESPONSE;
        }

        WARP_protocol::WARP_bssid_control_struct bssid_cntrl_struct;
        bssid_cntrl_struct.operation_code = BSSID_STATION_CLEAR_CODE;
        bssid_cntrl_struct.total_num_element = 0;
        if (parse_mac(json_string_value(bssid), bssid_cntrl_struct.bssid) != RETURN_CODE_OK)
        {
            json_decref(root);

            cout << "ERROR: invalid bssid format." << endl;
            this->set_error_msg(current_command, "Invalid mac address in json object.");
            return RETURN_CODE_ERROR | RETURN_CODE_SEND_RESPONSE;
        }

        WARP_protocol *bssid_clear_packet = WARP_protocol::create_bssid_control(&bssid_cntrl_struct);
        this->warp_to_wlan_agent.get()->sync(BSSID_NODE_OPS::SEND_BSSID_CNTRL, bssid_clear_packet);

        if (this->warp_to_wlan_agent.get()->timed_sync((int)BSSID_NODE_OPS::BSSID_CNTRL, &response, 500) == -1
            || response != BSSID_STATION_CLEAR_CODE)
        {
            delete bssid_clear_packet;
            json_decref(root);

            cout<< "ERROR: WARP failed to clear station associations, or the request timed out." << endl;
            this->set_error_msg(current_command, "WARP failed to clear station associations, or the request timed out.");
            return RETURN_CODE_ERROR | RETURN_CODE_SEND_RESPONSE;
        }

        // Clear successful
        delete bssid_clear_packet;

        // Construct mac address control packet to remove mac address
        WARP_protocol::WARP_mac_control_struct mac_address_cntrl_struct;
        mac_address_cntrl_struct.operation_code = MAC_REMOVE_CODE;

        // Parse Bssid
        if (parse_mac(json_string_value(bssid), mac_address_cntrl_struct.mac_address) != RETURN_CODE_OK)
        {
            json_decref(root);

            cout << "ERROR: invalid bssid format." << endl;
            this->set_error_msg(current_command, "Invalid mac address in json object.");
            return RETURN_CODE_ERROR | RETURN_CODE_SEND_RESPONSE;
        }

        // Parsing done. Time to send...
        WARP_protocol *mac_remove_packet = WARP_protocol::create_mac_control(&mac_address_cntrl_struct);
        this->warp_to_wlan_agent.get()->sync(BSSID_NODE_OPS::SEND_MAC_ADDR_CNTRL, mac_remove_packet);

        if (this->warp_to_wlan_agent.get()->timed_sync((int)BSSID_NODE_OPS::MAC_REMOVE, &response, 500) == -1
            || response != MAC_REMOVE_CODE)
        {
            delete mac_remove_packet;
            json_decref(root);

            cout<< "ERROR: WARP failed to remove mac address, or the request timed out." << endl;
            this->set_error_msg(current_command, "WARP failed to remove mac address, or the request timed out.");
            return RETURN_CODE_ERROR | RETURN_CODE_SEND_RESPONSE;
        }

        // Configuration succesful!
        delete mac_remove_packet;

        // Update corresponding bssid nodes
        this->update_bssids(BSSID_NODE_OPS::BSSID_REMOVE, (void*)json_string_value(bssid));
    }
    else if (strcmp(command_str, MAC_ASSOCIATE_CMD) == 0)
    {
        cout << "Associate command detected." << endl;
        // associate
        // Get changes object
        json_t *changes = json_object_get(root, JSON_CHANGES);
        if (!json_is_object(changes))
        {
            cout << "ERROR: no valid changes object found for mac associate." <<endl;

            json_decref(root);

            return RETURN_CODE_ERROR | RETURN_CODE_NO_RESPONSE;
        }

        // bssid - bssid of the virutal interface
        // mac_addr - mac addr to be associated
        json_t *bssid, *mac_addr;
        bssid = json_object_get(changes, JSON_BSSID);
        if (!json_is_string(bssid) )
        {
            cout << "ERROR: bssid is not a valid string for mac associate." <<endl;

            json_decref(root);

            return RETURN_CODE_ERROR | RETURN_CODE_NO_RESPONSE;
        }

        mac_addr = json_object_get(changes, JSON_MAC_ADDRESS);
        if (!json_is_string(mac_addr))
        {
            cout << "ERROR: mac address is not a valid string for mac associate." <<endl;

            json_decref(root);

            return RETURN_CODE_ERROR | RETURN_CODE_NO_RESPONSE;
        }

        WARP_protocol::WARP_bssid_control_struct* bssid_cntrl = (WARP_protocol::WARP_bssid_control_struct*)malloc(sizeof(WARP_protocol::WARP_bssid_control_struct));
        bssid_cntrl->total_num_element = 1;
        bssid_cntrl->operation_code = BSSID_STATION_ASSOCIATE_CODE;

        if (parse_mac(json_string_value(bssid), bssid_cntrl->bssid) != RETURN_CODE_OK)
        {
            json_decref(root);

            cout << "ERROR: invalid bssid format for mac associate." << endl;
            return RETURN_CODE_ERROR | RETURN_CODE_NO_RESPONSE;
        }

        bssid_cntrl->mac_addr = (uint8_t(*)[6])malloc(1 * sizeof(uint8_t[6]));

        if (parse_mac(json_string_value(mac_addr), bssid_cntrl->mac_addr[0]) != RETURN_CODE_OK)
        {
            json_decref(root);

            cout << "ERROR: invalid mac address format for mac associate." << endl;
            return RETURN_CODE_ERROR | RETURN_CODE_NO_RESPONSE;
        }

        // Everything successful. send packet
        cout << "--------------------------------------------------------------Sending associate\n";
        WARP_protocol *bssid_packet = WARP_protocol::create_bssid_control(bssid_cntrl);
        this->warp_to_wlan_agent.get()->sync(BSSID_NODE_OPS::SEND_BSSID_CNTRL, bssid_packet);

        if (this->warp_to_wlan_agent.get()->timed_sync((int)BSSID_NODE_OPS::BSSID_CNTRL, &response, 500) == -1
            || !(response == BSSID_STATION_ASSOCIATE_CODE || response == BSSID_STATION_EXISTED_CODE))
        {
            delete bssid_packet;
            free(bssid_cntrl->mac_addr);
            free(bssid_cntrl);
            json_decref(root);

            cout << "ERROR: WARP failed to associate mac address, or the request timed out." << endl;
            return RETURN_CODE_ERROR | RETURN_CODE_NO_RESPONSE;
        }

        delete bssid_packet;
        free(bssid_cntrl->mac_addr);
        free(bssid_cntrl);

        std::string to_associate(std::string((char*)json_string_value(bssid)) + "|" + std::string((char*)json_string_value(mac_addr)));

        // Update corresponding bssid nodes
        this->update_bssids(BSSID_NODE_OPS::BSSID_MAC_ASSOCIATE, (void*)to_associate.c_str());
    }
    else if (strcmp(command_str, MAC_DISASSOCIATE_CMD) == 0)
    {
        // disassociate
        // Get changes object
        json_t *changes = json_object_get(root, JSON_CHANGES);
        if (!json_is_object(changes))
        {
            cout << "ERROR: no valid changes object found." <<endl;

            json_decref(root);

            return RETURN_CODE_ERROR | RETURN_CODE_NO_RESPONSE;
        }

        // bssid - bssid of the virutal interface
        // mac_addr - mac addr to be associated
        json_t *bssid, *mac_addr;
        bssid = json_object_get(changes, JSON_BSSID);
        if (!json_is_string(bssid) )
        {
            cout << "ERROR: bssid is not a valid string for mac disassociate." <<endl;

            json_decref(root);

            return RETURN_CODE_ERROR | RETURN_CODE_NO_RESPONSE;
        }

        mac_addr = json_object_get(changes, JSON_MAC_ADDRESS);
        if (!json_is_string(mac_addr))
        {
            cout << "ERROR: mac address is not a valid string for mac disassociate." <<endl;

            json_decref(root);

            return RETURN_CODE_ERROR | RETURN_CODE_NO_RESPONSE;
        }

        WARP_protocol::WARP_bssid_control_struct* bssid_cntrl = (WARP_protocol::WARP_bssid_control_struct*)malloc(sizeof(WARP_protocol::WARP_bssid_control_struct));
        bssid_cntrl->total_num_element = 1;
        bssid_cntrl->operation_code = BSSID_STATION_DISASSOCIATE_CODE;

        if (parse_mac(json_string_value(bssid), bssid_cntrl->bssid) != RETURN_CODE_OK)
        {
            json_decref(root);

            cout << "ERROR: invalid bssid format for mac disassociate." << endl;
            return RETURN_CODE_ERROR | RETURN_CODE_NO_RESPONSE;
        }

        bssid_cntrl->mac_addr = (uint8_t(*)[6])malloc(1 * sizeof(uint8_t[6]));

        if (parse_mac(json_string_value(mac_addr), bssid_cntrl->mac_addr[0]) != RETURN_CODE_OK)
        {
            json_decref(root);

            cout << "ERROR: invalid mac address format for mac disassociate." << endl;
            return RETURN_CODE_ERROR | RETURN_CODE_NO_RESPONSE;
        }

        // Everything successful. send packet
        WARP_protocol *bssid_packet = WARP_protocol::create_bssid_control(bssid_cntrl);
        this->warp_to_wlan_agent.get()->sync(BSSID_NODE_OPS::SEND_BSSID_CNTRL, bssid_packet);

        if (this->warp_to_wlan_agent.get()->timed_sync((int)BSSID_NODE_OPS::BSSID_CNTRL, &response, 500) == -1
            || !(response == BSSID_STATION_DISASSOCIATE_CODE || response == BSSID_STATION_NOT_EXISTED_CODE))
        {
            delete bssid_packet;
            free(bssid_cntrl->mac_addr);
            free(bssid_cntrl);
            json_decref(root);

            cout << "ERROR: WARP failed to disassociate the mac address, or the request timed out." << endl;
            return RETURN_CODE_ERROR | RETURN_CODE_NO_RESPONSE;
        }

        delete bssid_packet;
        free(bssid_cntrl->mac_addr);
        free(bssid_cntrl);

        std::string to_disassociate(std::string((char*)json_string_value(bssid)) + "|" + std::string((char*)json_string_value(mac_addr)));

        // Update corresponding bssid nodes
        this->update_bssids(BSSID_NODE_OPS::BSSID_MAC_DISASSOCIATE, (void*)to_disassociate.c_str());
    }
    else if (strcmp(command_str, SHUTDOWN_CMD) == 0)
    {
        // Shutdown everything
        if_send_response = RETURN_CODE_SEND_RESPONSE;

        std::string shutdown_command(LEFT_BRACKET + QUOTE + "command" + QUOTE + COLON + SPACE + QUOTE + "shutdown" + RIGHT_BRACKET);
        set_msg(shutdown_command);

        // Update corresponding bssid nodes
        this->update_bssids(BSSID_NODE_OPS::COMMAND_SHUTDOWN, NULL);
    }

    json_decref(root);

    cout << "Parsing: Everything ok." << endl;

    // Determine whether to generate success reply or not
    if (if_send_response == RETURN_CODE_SEND_RESPONSE)
    {
        this->set_success_msg(current_command, current_radio);
    }

    return RETURN_CODE_OK | if_send_response;
}

void CommsAgent::set_error_msg(const std::string& command, const std::string& message)
{
    this->set_msg(string(RESPONSE_HEADER) + LEFT_BRACKET + QUOTE + "command" + QUOTE + COLON + SPACE + QUOTE + command + QUOTE
                                        + COMMA + SPACE + QUOTE + "changes" + QUOTE + COLON + SPACE + LEFT_BRACKET
                                        + QUOTE + "success" + QUOTE + COLON + SPACE + "false"
                                        + COMMA + SPACE + QUOTE + "error" + QUOTE + COLON + SPACE + QUOTE + message + QUOTE
                                        + RIGHT_BRACKET + RIGHT_BRACKET);
}

void CommsAgent::set_success_msg(const std::string& command, const std::string& radio, const std::string& message)
{
    this->set_msg(string(RESPONSE_HEADER) + LEFT_BRACKET + QUOTE + "command" + QUOTE + COLON + SPACE + QUOTE + command + QUOTE
                                        + COMMA + SPACE + QUOTE + "radio" + QUOTE + COLON + SPACE + QUOTE + radio + QUOTE
                                        + COMMA + SPACE + QUOTE + "changes" + QUOTE + COLON + SPACE + LEFT_BRACKET
                                        + QUOTE + "success" + QUOTE + COLON + SPACE + "true"
                                        + COMMA + SPACE + QUOTE + "error" + QUOTE + COLON + SPACE + QUOTE + message + QUOTE
                                        + RIGHT_BRACKET + RIGHT_BRACKET);
}

void CommsAgent::set_msg(const std::string& message)
{
    this->message_lock.lock();
    this->send_message.reset(new string(message));
    this->message_lock.unlock();
}

void CommsAgent::send_msg(const std::string& message)
{
    this->set_msg(message);
    sem_post(&this->signal);
}

void CommsAgent::update_bssids(int operation_code, void* bssid)
{
    cout << "Calling update_bssids with op: " << operation_code << endl;
    this->bssid_update_group_mux.lock();

    for (int i = 0; i < this->bssid_update_group.size(); i++)
    {
        this->bssid_update_group[i]->sync(operation_code, bssid);
    }

    this->bssid_update_group_mux.unlock();
}

void CommsAgent::add_to_bssid_group(BssidNode* node)
{
    this->bssid_update_group_mux.lock();

    this->bssid_update_group.push_back(node);

    this->bssid_update_group_mux.unlock();
}

void CommsAgent::set_warp_to_wlan_agent(BssidNode* agent)
{
    this->warp_to_wlan_agent.reset(agent);
}

int CommsAgent::sync(int operation_code, void* command)
{
    BSSID_NODE_OPS op = (BSSID_NODE_OPS)operation_code;
    switch(op)
    {
        case BSSID_NODE_OPS::COMMAND_ADD:
            this->command_queue_lock.lock();
            this->command_queue.push(string((char*)command));
            this->command_queue_lock.unlock();
            sem_post(&this->new_command);
    }

    return 0;
}

// Static variable init
int AgentFactory::current_thread_id = 0;
mutex AgentFactory::lock;
std::map<int, std::shared_ptr<RelayAgent>> AgentFactory::agent_threads;

void AgentFactory::spawn_agent_thread(vector<string>& args)
{
    string agent_type = args[0];
    args.erase(args.begin());
    if (agent_type.compare(WLAN_TO_WARP) == 0)
    {
        shared_ptr<WlanToWarpAgent> agent = make_shared<WlanToWarpAgent>();
        thread agent_thread(&WlanToWarpAgent::run, agent.get(), args);

        AgentFactory::lock.lock();
        AgentFactory::agent_threads.insert(pair<int, shared_ptr<RelayAgent>>(AgentFactory::current_thread_id, shared_ptr<RelayAgent>(agent)));
        AgentFactory::current_thread_id++;
        AgentFactory::lock.unlock();

        agent_thread.detach();
    }
    else if (agent_type.compare(MON_TO_WARP) == 0)
    {
        shared_ptr<MonToWarpAgent> agent = make_shared<MonToWarpAgent>();
        thread agent_thread(&MonToWarpAgent::run, agent.get(), args);

        AgentFactory::lock.lock();
        AgentFactory::agent_threads.insert(pair<int, shared_ptr<RelayAgent>>(AgentFactory::current_thread_id, shared_ptr<RelayAgent>(agent)));
        AgentFactory::current_thread_id++;
        AgentFactory::lock.unlock();

        agent_thread.detach();
    }
    else if (agent_type.compare(WARP_TO_WLAN) == 0)
    {
        shared_ptr<WarpToWlanAgent> agent = make_shared<WarpToWlanAgent>();
        thread agent_thread(&WarpToWlanAgent::run, agent.get(), args);

        AgentFactory::lock.lock();
        AgentFactory::agent_threads.insert(pair<int, shared_ptr<RelayAgent>>(AgentFactory::current_thread_id, shared_ptr<RelayAgent>(agent)));
        AgentFactory::current_thread_id++;
        AgentFactory::lock.unlock();

        agent_thread.detach();
    }
}

void AgentFactory::kill_agent_thread(int thread_id)
{
    AgentFactory::lock.lock();

    AgentFactory::agent_threads[thread_id].get()->signal_complete();
    AgentFactory::agent_threads[thread_id].reset();
    AgentFactory::agent_threads.erase(thread_id);

    AgentFactory::lock.unlock();
}
