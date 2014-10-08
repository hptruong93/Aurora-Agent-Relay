#include <thread>
#include <iostream>
#include <vector>
#include <stdio.h>
#include <string>

#include <zmq.hpp>
#include <jansson.h>

#include "mon_to_warp_agent.h"
#include "wlan_to_warp_agent.h"
#include "warp_to_wlan_agent.h"
#include "comms_agent.h"
#include "agents.h"

using namespace RelayAgents;
using namespace std;

ParseFunctionCode parse_input(string input, vector<string>& tokens)
{
    int pos;
    while((pos = input.find(" ")) != string::npos)
    {
        string arg = input.substr(0, pos);
        tokens.push_back(arg);
        input.erase(0, pos + 1);
    }

    tokens.push_back(input);

    string command = tokens[0];
    if (command.compare(HELP_COMMAND) == 0)
    {
        // Help function
        return ParseFunctionCode::HELP;
    }
    else if ((command.compare(WLAN_TO_WARP) == 0) ||
                (command.compare(MON_TO_WARP) == 0) ||
                (command.compare(WARP_TO_WLAN) == 0))
    {
        // Start threads
        return ParseFunctionCode::NEW_THREAD;
    }
    else
    {
        cout<<"ERROR: Unrecognized command"<<endl;

        return ParseFunctionCode::ERROR;
    }
}

AgentFactory::AgentFactory()
{

}

void AgentFactory::spawn_agent_thread(vector<string>& args)
{
    string agent_type = args[0];
    args.erase(args.begin());
    if (agent_type.compare(WLAN_TO_WARP) == 0)
    {
        WlanToWarpAgent agent;
        thread agent_thread(&WlanToWarpAgent::run, &agent, ref(args));

        agent_thread.detach();
    }
    else if (agent_type.compare(MON_TO_WARP) == 0)
    {
        MonToWarpAgent agent;
        thread agent_thread(&MonToWarpAgent::run, &agent, ref(args));

        agent_thread.detach();
    }
    else if (agent_type.compare(WARP_TO_WLAN) == 0)
    {
        WarpToWlanAgent agent;
        thread agent_thread(&WarpToWlanAgent::run, &agent, ref(args));

        agent_thread.detach();
    }
}

int main(int argc, char *argv[])
{
    if (argc < 4)
    {
        cout<<"Invalid number of arguments.Need out interface, send port and receive port."<<endl;
    }

    CommsAgent comms_agent(argv[1], argv[2], argv[3]);
    thread comms_thread(&CommsAgent::spin, &comms_agent);
    comms_thread.detach();

    string input_string;
    AgentFactory factory;

    // Main thread. Manually spawn agent threads
    while(true)
    {
        vector<string> args;
        cout<<"Enter command:"<<endl;
        getline(cin, input_string);

        if (parse_input(input_string, args) == ParseFunctionCode::NEW_THREAD)
        {
            factory.spawn_agent_thread(args);
        }
    }

    return 0;
}