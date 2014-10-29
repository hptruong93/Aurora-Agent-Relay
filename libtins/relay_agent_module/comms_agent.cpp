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

CommsAgent::CommsAgent(const char *init_send_port, const char *init_recv_port)
{
    send_port = unique_ptr<string>(new string(init_send_port));
    recv_port = unique_ptr<string>(new string(init_recv_port));

    // Consume the semaphore
    sem_init(&this->signal, 0, 1);
    sem_wait(&this->signal);
}

void CommsAgent::send_loop()
{
    zmq::message_t send_message(DEFAULT_MSG_SIZE);

    zmq::context_t ctx(1);
    zmq::socket_t pub_socket = zmq::socket_t(ctx, ZMQ_PUB);


    string pub_address = string("tcp://*:") + string(this->send_port.get()->c_str());
    pub_socket.bind(pub_address.c_str());

    while (true)
    {
        // cout<<"waiting..."<<endl;
        sem_wait(&this->signal);
        this->message_lock.lock();

        snprintf((char*)send_message.data(), DEFAULT_MSG_SIZE, this->send_message.get()->c_str());
        send_message.rebuild();
        pub_socket.send(send_message);

        this->message_lock.unlock();
    }
}

void CommsAgent::recv_loop()
{
    zmq::message_t received_msg;

    zmq::context_t ctx(1);
    zmq::socket_t sub_socket = zmq::socket_t(ctx, ZMQ_SUB);

    string sub_address = string("tcp://localhost:") + string(this->recv_port.get()->c_str());
    sub_socket.connect(sub_address.c_str());

    // Set socket options for receive socket
    // Receive everything for now
    sub_socket.setsockopt(ZMQ_SUBSCRIBE, "", 0);

    while(true)
    {
        sub_socket.recv(&received_msg);
        // Do parsing
        this->parse_json((char*)received_msg.data());
    }
}

ErrorCode CommsAgent::parse_json(const char *json_string)
{
    json_t *root;
    json_error_t error;

    root = json_loads(json_string, 0, &error);

    if (!root)
    {
        cout<<"ERROR: on line "<<error.line<<": "<<error.text<<endl;

        this->set_error_msg();
        return ErrorCode::ERROR;
    }

    // Check if is array
    if (!json_is_array(root))
    {
        cout<<"ERROR: json root is not array"<<endl;
        json_decref(root);

        this->set_error_msg();
        return ErrorCode::ERROR;
    }

    char *first_json;

    for(int i = 0; i < json_array_size(root); i++)
    {
        json_t *data, *mac_addr, *channel, *hwmode, *txpower, *disabled;

        data = json_array_get(root, i);
        // Send back the first data
        if (i == 0) {
            first_json = json_dumps(data, 0);

            #ifndef TEST_JSON_DECODER
            
            this->message_lock.lock();

            this->send_message.reset(new string(first_json));

            this->message_lock.unlock();

            #endif

            free(first_json);
        }

        if (!json_is_object(data))
        {
            cout<<"ERROR: Corrupt Json object at index "<<i<<endl;
            json_decref(root);

            this->set_error_msg();
            return ErrorCode::ERROR;
        }

        mac_addr = json_object_get(data, MAC_ADDRESS);
        if (!json_is_string(mac_addr))
        {
            cout<<"ERROR: Corrupt mac address for json object at index "<<i<<endl;
            json_decref(root);

            this->set_error_msg();
            return ErrorCode::ERROR;
        }

        channel = json_object_get(data, CHANNEL);
        if (!json_is_integer(channel))
        {
            cout<<"ERROR: Corrupt channel for json object at index "<<i<<endl;
            json_decref(root);

            this->set_error_msg();
            return ErrorCode::ERROR;
        }

        hwmode = json_object_get(data, HW_MODE);
        if (!json_is_integer(hwmode))
        {
            cout<<"ERROR: Corrupt hwmode for json object at index "<<i<<endl;
            json_decref(root);

            this->set_error_msg();
            return ErrorCode::ERROR;
        }

        txpower = json_object_get(data, TX_POWER);
        if (!json_is_integer(txpower))
        {
            cout<<"ERROR: Corrupt txpower for json object at index "<<i<<endl;
            json_decref(root);

            this->set_error_msg();
            return ErrorCode::ERROR;
        }

        disabled = json_object_get(data, DISABLED);
        if (!json_is_integer(disabled))
        {
            cout<<"ERROR: Corrupt disabled for json object at index "<<i<<endl;
            json_decref(root);

            this->set_error_msg();
            return ErrorCode::ERROR;
        }

        // Parsing done

        // Mac Address Control Packet
        WARP_protocol::WARP_mac_control_struct mac_address_cntrl_struct;
        mac_address_cntrl_struct.operation_code = MAC_ADD_CODE;
        if (parse_mac(json_string_value(mac_addr), mac_address_cntrl_struct.mac_address) != ErrorCode::OK)
        {
            this->set_error_msg();
            return ErrorCode::ERROR;
        }
        WARP_protocol *mac_add_packet = WARP_protocol::create_mac_control(&mac_address_cntrl_struct);
        this->warp_to_wlan_agent.get()->sync(BSSID_NODE_OPS::SEND_MAC_ADDR_CNTRL, mac_add_packet);
        delete mac_add_packet;

        // Wait until WARP talks back
        int error;
        if ((error = this->warp_to_wlan_agent.get()->timed_sync((int)BSSID_NODE_OPS::MAC_ADD, 500)) == -1)
        {
            // TODO: Set error message to be sent back to Al's python code
            this->set_error_msg();
            return ErrorCode::ERROR;
        }

        // Transmission control packet
        WARP_protocol::WARP_transmission_control_struct transmission_cntrl_struct;
        transmission_cntrl_struct.disabled = (uint8_t) json_integer_value(disabled);
        transmission_cntrl_struct.tx_power = (uint8_t) json_integer_value(txpower);
        transmission_cntrl_struct.channel = (uint8_t) json_integer_value(channel);
        transmission_cntrl_struct.rate = 1;
        transmission_cntrl_struct.hw_mode = (uint8_t) json_integer_value(hwmode);

        if (parse_mac(json_string_value(mac_addr), transmission_cntrl_struct.bssid) != ErrorCode::OK)
        {
            // TODO: Set error message to be sent back to Al's python code
            this->set_error_msg();
            return ErrorCode::ERROR;
        }

        #ifdef TEST_JSON_DECODER

        cout<<"Mac Address: "<<std::hex<<(int)transmission_cntrl_struct.bssid[0]<<":"<<(int)transmission_cntrl_struct.bssid[1]<<":"
                                <<(int)transmission_cntrl_struct.bssid[2]<<":"<<(int)transmission_cntrl_struct.bssid[3]<<":"
                                <<(int)transmission_cntrl_struct.bssid[4]<<":"
                                <<(int)transmission_cntrl_struct.bssid[5]<<endl;
        cout<<"Channel: "<<(int)transmission_cntrl_struct.channel<<endl;

        #else

        WARP_protocol *transmission_packet = WARP_protocol::create_transmission_control(&transmission_cntrl_struct);
        this->warp_to_wlan_agent.get()->sync(BSSID_NODE_OPS::SEND_TRANSMISSION_CNTRL, transmission_packet);
        delete transmission_packet;

        if ((error = this->warp_to_wlan_agent.get()->timed_sync((int)BSSID_NODE_OPS::TRANSMISSION_CNTRL, 500)) == -1)
        {
            // TODO: Set error message to be sent back to Al's python code
            this->set_error_msg();
            return ErrorCode::ERROR;
        }

        #endif
    }

    // Release the semaphore so that we can talk back to Al
    sem_post(&this->signal);

    json_decref(root);

    return ErrorCode::OK;
}

void CommsAgent::set_error_msg()
{
    this->message_lock.lock();
    this->send_message.reset(new string("{ Failed: true }"));
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