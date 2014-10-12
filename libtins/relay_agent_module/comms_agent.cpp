#include <thread>
#include <iostream>

#include "mon_to_warp_agent.h"
#include "wlan_to_warp_agent.h"
#include "warp_to_wlan_agent.h"
#include "relay_agent.h"
#include "comms_agent.h"

using namespace RelayAgents;
using namespace std;

ErrorCode parse_mac(const char* origin, uint8_t dest[])
{
    if (sscanf(origin, "%02c:%02c:%02c:%02c:%02c:%02c", &dest[0], &dest[1], &dest[2], &dest[3], &dest[4], &dest[5]) != 6)
    {
        cout<<"ERROR: Failed to parse mac address string into uint arrays"<<endl;

        return ErrorCode::ERROR;
    }

    return ErrorCode::OK;
}

CommsAgent::CommsAgent(const char *init_out_interface, string init_send_port, string init_recv_port) 
                : send_port(init_send_port), recv_port(init_recv_port)
{
    this->out_interface = unique_ptr<string>(new string(init_out_interface));
    this->protocol_sender = unique_ptr<WARP_ProtocolSender>(new WARP_ProtocolSender(new PacketSender(init_out_interface)));

    // ZMQ settings
    this->context = unique_ptr<zmq::context_t>(new zmq::context_t(1));
    this->pub_socket = unique_ptr<zmq::socket_t>(new zmq::socket_t(*(this->context.get()), ZMQ_PUB));
    this->sub_socket = unique_ptr<zmq::socket_t>(new zmq::socket_t(*(this->context.get()), ZMQ_SUB));

    string pub_address = "tcp://*:" + this->send_port;
    this->pub_socket.get()->bind(pub_address.c_str());

    string sub_address = "tcp://*:" + this->recv_port;
    this->sub_socket.get()->connect(sub_address.c_str());

    // Set socket options for receive socket
    // Receive everything for now
    this->sub_socket.get()->setsockopt(ZMQ_SUBSCRIBE, "", 0);
}

void CommsAgent::spin()
{
    zmq::message_t received_msg;

    while(true)
    {
        this->sub_socket.get()->recv(&(received_msg));

        // Do parsing
        this->parse_json((char*)received_msg.data());
    }
}

ErrorCode CommsAgent::parse_json(const char *json_string)
{
    json_t *root;
    json_error_t error;

    zmq::message_t send_message(DEFAULT_MSG_SIZE);

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
            snprintf((char*)send_message.data(), DEFAULT_MSG_SIZE, first_json);
            send_message.rebuild();
            this->pub_socket.get()->send(send_message);
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

        WARP_protocol *packet = WARP_protocol::create_transmission_control(&transmission_cntrl_struct);
        this->protocol_sender.get()->send(*packet, TYPE_CONTROL, SUBTYPE_TRANSMISSION_CONTROL);
        delete packet;
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