#include <thread>
#include <iostream>
#include <vector>
#include <string>
#include <stdio.h>

#include "mon_to_warp_agent.h"
#include "wlan_to_warp_agent.h"
#include "warp_to_wlan_agent.h"
#include "comms_agent.h"
#include "agents.h"
// Testing
#include "test.h"

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
    else if (command.compare(KILL_COMMAND) == 0)
    {
        // Kill a thread
        return ParseFunctionCode::KILL;
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

int main(int argc, char *argv[])
{
    #ifdef TEST_JSON_DECODER

    CommsAgent comms_agent;
    comms_agent.parse_json(SAMPLE_JSON_STRING);

    #else

    unique_ptr<CommsAgent> comms_agent;

    if (argc < 4)
    {
        // Comms Agent
        comms_agent = unique_ptr<CommsAgent>(new CommsAgent(NULL, "5555", "5556"));
        thread comms_receive_thread(&CommsAgent::spin, comms_agent.get());
        thread comms_send_thread(&CommsAgent::send, comms_agent.get());
        comms_receive_thread.detach();
        comms_send_thread.detach();

        //comms_agent.send();
    }
    else
    {
        // Comms Agent
        comms_agent = unique_ptr<CommsAgent>(new CommsAgent(argv[1], argv[2], argv[3]));
        thread comms_receive_thread(&CommsAgent::spin, comms_agent.get());
        thread comms_send_thread(&CommsAgent::send, comms_agent.get());
        comms_receive_thread.detach();
        comms_send_thread.detach();
    }

    // Agent Factory
    string input_string;

    // Main thread. Manually spawn agent threads
    while(true)
    {
        vector<string> args;
        cout<<"Enter command:"<<endl;
        getline(cin, input_string);

        ParseFunctionCode code = parse_input(input_string, args);
        if (code == ParseFunctionCode::NEW_THREAD)
        {
            AgentFactory::spawn_agent_thread(args);
        }
        else if (code == ParseFunctionCode::KILL)
        {
            AgentFactory::kill_agent_thread(stoi(args[1]));
        }
    }

    #endif

    return 0;
}