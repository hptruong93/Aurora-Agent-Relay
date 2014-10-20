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

// Static variable
sem_t CommsAgent::signal;
mutex CommsAgent::message_lock;
unique_ptr<string> CommsAgent::send_message;

CommsAgent::CommsAgent(const char *init_out_interface, const char *init_send_port, const char *init_recv_port)
{
    send_port = unique_ptr<const char>(init_send_port);
    recv_port = unique_ptr<const char>(init_recv_port);

    sem_init(&CommsAgent::signal, 0, 1);
    sem_wait(&CommsAgent::signal);
    #ifndef TEST_JSON_DECODER
    if (init_out_interface != NULL)
    {
        this->out_interface = unique_ptr<string>(new string(init_out_interface));
        this->protocol_sender = unique_ptr<WARP_ProtocolSender>(new WARP_ProtocolSender(new PacketSender(init_out_interface)));
    }

    #endif
}

void CommsAgent::set_out_interface(const char *new_out_interface)
{
    this->out_interface.reset(new string(new_out_interface));
    this->protocol_sender.reset(new WARP_ProtocolSender(new PacketSender(new_out_interface)));
}

void CommsAgent::send()
{
    zmq::message_t send_message(DEFAULT_MSG_SIZE);

    zmq::context_t ctx(1);
    zmq::socket_t pub_socket = zmq::socket_t(ctx, ZMQ_PUB);


    string pub_address = string("tcp://*:") + string(this->send_port.get());
    pub_socket.bind(pub_address.c_str());

    while (true)
    {
        // cout<<"waiting..."<<endl;
        sem_wait(&CommsAgent::signal);
        CommsAgent::message_lock.lock();

        snprintf((char*)send_message.data(), DEFAULT_MSG_SIZE, CommsAgent::send_message.get()->c_str());
        send_message.rebuild();
        pub_socket.send(send_message);

        CommsAgent::message_lock.unlock();
    }
}

void CommsAgent::spin()
{
    zmq::message_t received_msg;

    zmq::context_t ctx(1);
    zmq::socket_t sub_socket = zmq::socket_t(ctx, ZMQ_SUB);

    string sub_address = string("tcp://localhost:") + string(this->recv_port.get());
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

        return ErrorCode::ERROR;
    }

    // Check if is array
    if (!json_is_array(root))
    {
        cout<<"ERROR: json root is not array"<<endl;
        json_decref(root);

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
            
            CommsAgent::message_lock.lock();

            CommsAgent::send_message.reset(new string(first_json));

            CommsAgent::message_lock.unlock();

            sem_post(&CommsAgent::signal);

            #endif

            free(first_json);
        }

        if (!json_is_object(data))
        {
            cout<<"ERROR: Corrupt Json object at index "<<i<<endl;
            json_decref(root);

            return ErrorCode::ERROR;
        }

        mac_addr = json_object_get(data, MAC_ADDRESS);
        if (!json_is_string(mac_addr))
        {
            cout<<"ERROR: Corrupt mac address for json object at index "<<i<<endl;
            json_decref(root);

            return ErrorCode::ERROR;
        }

        channel = json_object_get(data, CHANNEL);
        if (!json_is_integer(channel))
        {
            cout<<"ERROR: Corrupt channel for json object at index "<<i<<endl;
            json_decref(root);

            return ErrorCode::ERROR;
        }

        hwmode = json_object_get(data, HW_MODE);
        if (!json_is_integer(hwmode))
        {
            cout<<"ERROR: Corrupt hwmode for json object at index "<<i<<endl;
            json_decref(root);

            return ErrorCode::ERROR;
        }

        txpower = json_object_get(data, TX_POWER);
        if (!json_is_integer(txpower))
        {
            cout<<"ERROR: Corrupt txpower for json object at index "<<i<<endl;
            json_decref(root);

            return ErrorCode::ERROR;
        }

        disabled = json_object_get(data, DISABLED);
        if (!json_is_integer(disabled))
        {
            cout<<"ERROR: Corrupt disabled for json object at index "<<i<<endl;
            json_decref(root);

            return ErrorCode::ERROR;
        }

        // Parsing done
        WARP_protocol::WARP_transmission_control_struct transmission_cntrl_struct;
        transmission_cntrl_struct.disabled = (uint8_t) json_integer_value(disabled);
        transmission_cntrl_struct.tx_power = (uint8_t) json_integer_value(txpower);
        transmission_cntrl_struct.channel = (uint8_t) json_integer_value(channel);
        transmission_cntrl_struct.rate = 1;
        transmission_cntrl_struct.hw_mode = (uint8_t) json_integer_value(hwmode);

        if (parse_mac(json_string_value(mac_addr), transmission_cntrl_struct.bssid) != ErrorCode::OK)
        {
            return ErrorCode::ERROR;
        }

        #ifdef TEST_JSON_DECODER

        cout<<"Mac Address: "<<std::hex<<(int)transmission_cntrl_struct.bssid[0]<<":"<<(int)transmission_cntrl_struct.bssid[1]<<":"
                                <<(int)transmission_cntrl_struct.bssid[2]<<":"<<(int)transmission_cntrl_struct.bssid[3]<<":"
                                <<(int)transmission_cntrl_struct.bssid[4]<<":"
                                <<(int)transmission_cntrl_struct.bssid[5]<<endl;
        cout<<"Channel: "<<(int)transmission_cntrl_struct.channel<<endl;

        #else

        WARP_protocol *packet = WARP_protocol::create_transmission_control(&transmission_cntrl_struct);
        this->protocol_sender.get()->send(*packet, TYPE_CONTROL, SUBTYPE_TRANSMISSION_CONTROL);
        delete packet;

        #endif
    }

    json_decref(root);

    return ErrorCode::OK;
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