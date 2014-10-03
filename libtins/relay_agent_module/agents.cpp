#include "mon_to_warp_agent.h"
#include "wlan_to_warp_agent.h"
#include "warp_to_wlan_agent.h"
#include "agents.h"

#include <thread>
#include <iostream>

#include <zmq.hpp>
#include <jansson.h>

using namespace RelayAgents;
using namespace std;

AgentFactory::AgentFactory(string init_port) : port(init_port), util(AgentUtil())
{
    this->context = unique_ptr<zmq::context_t>(new zmq::context_t(1));
    this->socket = unique_ptr<zmq::socket_t>(new zmq::socket_t(*(this->context.get()), ZMQ_REP));

    string address = "tcp://*:" + this->port;
    (this->socket.get())->bind(address.c_str());
}

void AgentFactory::spin()
{
    zmq::message_t received_msg;
    zmq::message_t send_message(DEFAULT_MSG_SIZE);

    while(true)
    {
        this->socket.get()->recv(&(received_msg));

        // Do parsing
        if (this->util.parse_json((char*)received_msg.data()) == 0)
        {
            char *argv[4];
            int argc = 0;
            // Everything successfullly parsed
            argc = this->util.get_parameter_count();
            argv[0] = this->util.transfer_parameter(INTERFACE_IN);
            argv[1] = this->util.transfer_parameter(INTERFACE_OUT);
            if (argc > 2)
            {
                argv[2] = this->util.transfer_parameter(MAC_ADDRESS);
            }

            // Spawn thread
            {
                this->spawn_agent_thread(this->util.get_parameter(AGENT_TYPE), argc, argv);
            }
        }

        // TODO: Generate message to send back

        send_message.rebuild();
        this->socket.get()->send(send_message);
    }
}

void AgentFactory::spawn_agent_thread(const char *agent_type, int argc, char *argv[])
{
    if (strcmp(agent_type, WLAN_TO_WARP) == 0)
    {
        WlanToWarpAgent agent;
        thread agent_thread(&WlanToWarpAgent::run, &agent, argc, argv);

        agent_thread.detach();
    }
}

AgentUtil::AgentUtil() : in_interface(nullptr), out_interface(nullptr), mac_addr(nullptr), agent_type(nullptr), parameters(0)
{

}

int AgentUtil::parse_json(const char *json_string)
{
    this->reset();
    
    json_t *root;
    json_error_t error;

    root = json_loads(json_string, 0, &error);
    if (!root)
    {
        cout<<"ERROR: on line "<<error.line<<": "<<error.text<<endl;

        return 1;
    }

    // TOdO: Json Parsing

    json_decref(root);

    return 0;
}

int AgentUtil::get_parameter_count() const
{
    return this->parameters;
}

char* AgentUtil::transfer_parameter(const char *parameter_name)
{
    if (strcmp(parameter_name, INTERFACE_IN) == 0)
    {
        return this->in_interface.release();
    }
    else if (strcmp(parameter_name, INTERFACE_OUT) == 0)
    {
        return this->out_interface.release();
    }
    else if (strcmp(parameter_name, MAC_ADDRESS) == 0)
    {
        return this->mac_addr.release();
    }

    return NULL;
}

char* AgentUtil::get_parameter(const char *parameter_name) const
{
    if (strcmp(parameter_name, INTERFACE_IN) == 0)
    {
        return this->in_interface.get();
    }
    else if (strcmp(parameter_name, INTERFACE_OUT) == 0)
    {
        return this->out_interface.get();
    }
    else if (strcmp(parameter_name, MAC_ADDRESS) == 0)
    {
        return this->mac_addr.get();
    }
    else if (strcmp(parameter_name, AGENT_TYPE) == 0)
    {
        return this->agent_type.get();
    }

    return NULL;    
}

void AgentUtil::reset()
{
    this->in_interface.reset();
    this->out_interface.reset();
    this->mac_addr.reset();
    this->agent_type.reset();
    this->parameters = 0;
}

int main(int argc, char *argv[])
{
    // Test
    WlanToWarpAgent test_wtp_agent;
    thread t1(&WlanToWarpAgent::run, &test_wtp_agent, argc, argv);

    t1.join();

    return 0;
}