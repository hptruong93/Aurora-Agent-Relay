#include <thread>
#include <iostream>

#include "mon_to_warp_agent.h"
#include "wlan_to_warp_agent.h"
#include "warp_to_wlan_agent.h"
#include "relay_agent.h"
#include "comms_agent.h"
// Testing
#include "test.h"

using namespace RelayAgents;
using namespace std;

ErrorCode parse_mac(const char* origin, uint8_t dest[])
{
    if (sscanf(origin, "%02hhx:%02hhx:%02hhx:%02hhx:%02hhx:%02hhx", &dest[0], &dest[1], &dest[2], &dest[3], &dest[4], &dest[5]) != 6)
    {
        cout<<"ERROR: Failed to parse mac address string into uint arrays"<<endl;

        return ErrorCode::ERROR;
    }

    return ErrorCode::OK;
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


    string pub_address = string("tcp://*:") + *this->send_port.get();
    pub_socket.bind(pub_address.c_str());

    while (true)
    {
        zmq::message_t send_message(DEFAULT_MSG_SIZE);

        sem_wait(&this->signal);
        cout << "Trying to send something\n" << endl;
        this->message_lock.lock();

        snprintf((char*)send_message.data(), DEFAULT_MSG_SIZE, this->send_message.get()->c_str());
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

            this->parse_json(next_command.c_str());
            sem_post(&this->signal);

            this->command_queue_lock.lock();
            this->command_queue.pop();
            // cout<<"POPPED SOMETHING!"<<endl;
            // cout<<"SIZE AFTER POP: "<<this->command_queue.size()<<endl;
            this->command_queue_lock.unlock();
        }
    }
}

ErrorCode CommsAgent::parse_json(const char *json_string)
{
    json_t *root;
    json_error_t error;

    root = json_loads(json_string, 0, &error);

    if (!root)
    {
        if (strcmp(json_string, "test") == 0)
        {
            // Test string
            this->set_msg("{ success: True }");
            return ErrorCode::OK;
        }

        cout<<"ERROR: on line "<<error.line<<": "<<error.text<<endl;

        this->set_error_msg("Error parsing the json object.");
        return ErrorCode::ERROR;
    }

    // Set response message
    this->set_msg(string(json_string));

    // Test if root is a valid json object
    if (!json_is_object(root))
    {
        cout << "ERROR: root is not a json object." <<endl;
        this->set_error_msg("Error parsing the json object.");
        json_decref(root);

        return ErrorCode::ERROR;
    }

    // Get command type
    json_t *command = json_object_get(root, JSON_COMMAND);
    if (!json_is_string(command))
    {
        cout << "ERROR: command is not a string." <<endl;
        this->set_error_msg("Error parsing the json object.");

        json_decref(root);

        return ErrorCode::ERROR;
    }

    // Used for response messages
    uint8_t response;

    // Parse command
    const char *command_str = json_string_value(command);

    if (strcmp(command_str, RADIO_SET_CMD) == 0 || 
        strcmp(command_str, RADIO_BULK_SET_CMD) == 0)
    {
        // json_decref(command);
        // Get changes object
        json_t *changes = json_object_get(root, JSON_CHANGES);
        if (!json_is_object(changes))
        {
            cout << "ERROR: no valid changes object found." <<endl;
            this->set_error_msg("Error parsing the json object.");

            json_decref(root);

            return ErrorCode::ERROR;
        }

        // Parse the changes
        json_t *parameters, *bssid;
        bssid = json_object_get(changes, JSON_MAC_ADDRESS);
        if (!json_is_string(bssid) )
        {
            cout << "ERROR: bssid is not a valid string." <<endl;
            this->set_error_msg("Error parsing the json object.");

            json_decref(root);

            return ErrorCode::ERROR;
        }

        // Bssid parsing and send mac address control protocol
        WARP_protocol::WARP_mac_control_struct mac_address_cntrl_struct;
        mac_address_cntrl_struct.operation_code = MAC_ADD_CODE;
        if (parse_mac(json_string_value(bssid), mac_address_cntrl_struct.mac_address) != ErrorCode::OK)
        {
            json_decref(root);

            cout << "ERROR: invalid bssid format." << endl;
            this->set_error_msg("Invalid mac address in json object.");
            return ErrorCode::ERROR;
        }

        // Parsing done. We can send the mac address control now
        WARP_protocol *mac_add_packet = WARP_protocol::create_mac_control(&mac_address_cntrl_struct);
        this->warp_to_wlan_agent.get()->sync(BSSID_NODE_OPS::SEND_MAC_ADDR_CNTRL, mac_add_packet);

        // Wait until WARP talks back
        int error;
        if ((error = this->warp_to_wlan_agent.get()->timed_sync((int)BSSID_NODE_OPS::MAC_ADD, &response, 500)) == -1
            || response != MAC_ADD_CODE)
        {
            delete mac_add_packet;

            json_decref(root);

            cout << "ERROR: WARP failed to add mac address, or the request timed out." << endl;
            this->set_error_msg("WARP failed to add mac address, or the request timed out.");
            return ErrorCode::ERROR;
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

            free(transmission_control);
            json_decref(root);

            return ErrorCode::ERROR;
        }
        transmission_control->channel = (uint8_t)json_integer_value(parameters);

        parameters = json_object_get(changes, JSON_HW_MODE);
        if (!json_is_string(parameters))
        {
            cout << "ERROR: invalid hw_mode format." << endl;

            free(transmission_control);
            json_decref(root);

            return ErrorCode::ERROR;
        }
        string hwmode_str(json_string_value(parameters));
        transmission_control->hw_mode = parse_hwmode(hwmode_str);

        parameters = json_object_get(changes, JSON_TX_POWER);
        if (!json_is_integer(parameters))
        {
            cout << "ERROR: invalid tx_power format." << endl;

            free(transmission_control);
            json_decref(root);

            return ErrorCode::ERROR;
        }
        transmission_control->tx_power = (uint8_t)json_integer_value(parameters);

        parameters = json_object_get(changes, JSON_DISABLED);
        if (!json_is_integer(parameters))
        {
            cout << "ERROR: invalid disabled format." << endl;

            free(transmission_control);
            json_decref(root);

            return ErrorCode::ERROR;
        }
        transmission_control->disabled = (uint8_t)json_integer_value(parameters);

        // Done! Time to send
        WARP_protocol *transmission_packet = WARP_protocol::create_transmission_control(transmission_control);
        this->warp_to_wlan_agent.get()->sync(BSSID_NODE_OPS::SEND_TRANSMISSION_CNTRL, transmission_packet);

        if ((error = this->warp_to_wlan_agent.get()->timed_sync((int)BSSID_NODE_OPS::TRANSMISSION_CNTRL, &response, 500)) == -1
            || response != TRANSMISSION_CONFIGURE_SUCCESS_CODE)
        {
            delete transmission_packet;
            free(transmission_control);
            json_decref(root);

            cout << "ERROR: WARP failed to set up the configuration requested, or the request timed out." << endl;
            this->set_error_msg("WARP failed to set up the configuration requested, or the request timed out.");
            return ErrorCode::ERROR;
        }

        // Configuration successful!!
        delete transmission_packet;
        free(transmission_control);
        
        // Update corresponding bssid nodes
        this->update_bssids(BSSID_NODE_OPS::BSSID_REMOVE, (void*)json_string_value(bssid));
    }
    else if (strcmp(command_str, UCI_DELETE_SECTION) == 0 ||
                strcmp(command_str, UCI_DELETE_BSS) == 0)
    {
        // Get Bssid
        json_t *bssid;
        bssid = json_object_get(root, JSON_MAC_ADDRESS);
        if (!json_is_string(bssid))
        {
            cout << "ERROR: bssid is not a valid string." <<endl;
            this->set_error_msg("Error parsing the json object.");

            json_decref(root);

            return ErrorCode::ERROR;
        }

        // Construct mac address control packet to remove mac address
        WARP_protocol::WARP_mac_control_struct mac_address_cntrl_struct;
        mac_address_cntrl_struct.operation_code = MAC_REMOVE_CODE;

        // Parse Bssid
        if (parse_mac(json_string_value(bssid), mac_address_cntrl_struct.mac_address) != ErrorCode::OK)
        {
            json_decref(root);

            cout << "ERROR: invalid bssid format." << endl;
            this->set_error_msg("Invalid mac address in json object.");
            return ErrorCode::ERROR;
        }

        // Parsing done. Time to send...
        WARP_protocol *mac_remove_packet = WARP_protocol::create_mac_control(&mac_address_cntrl_struct);
        this->warp_to_wlan_agent.get()->sync(BSSID_NODE_OPS::SEND_MAC_ADDR_CNTRL, mac_remove_packet);

        uint8_t response;
        int error;
        if ((error = this->warp_to_wlan_agent.get()->timed_sync((int)BSSID_NODE_OPS::MAC_REMOVE, &response, 500)) == -1
            || response != MAC_REMOVE_CODE)
        {
            delete mac_remove_packet;
            json_decref(root);

            cout<< "ERROR: WARP failed to remove mac address, or the request timed out." << endl;
            this->set_error_msg("WARP failed to remove mac address, or the request timed out.");
            return ErrorCode::ERROR;
        }

        // Configuration succesful!
        delete mac_remove_packet;

        // Update corresponding bssid nodes
        this->update_bssids(BSSID_NODE_OPS::BSSID_REMOVE, (void*)json_string_value(bssid));
    }

    json_decref(root);

    cout << "Parsing: Everything ok." << endl;

    return ErrorCode::OK;
}

void CommsAgent::set_error_msg(const std::string& message)
{
    this->message_lock.lock();
    this->send_message.reset(new string("{ successful: False,  message: \"" + message + "\"}"));
    this->message_lock.unlock();
}

void CommsAgent::set_msg(const std::string& message)
{
    this->message_lock.lock();
    this->send_message.reset(new string(message));
    this->message_lock.unlock();
}

void CommsAgent::update_bssids(int operation_code, void* bssid)
{
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
