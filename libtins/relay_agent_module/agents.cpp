#include <thread>
#include <iostream>
#include <vector>
#include <string>
#include <stdio.h>

#include "mon_to_warp_agent.h"
#include "wlan_to_warp_agent.h"
#include "warp_to_wlan_agent.h"
#include "comms_agent.h"
#include "dpm_agent.h"
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

// Input parameters:
// argv[1]: zmq send port
// argv[2]: zmq receive port
// argv[3]: zmq peer ip address
int main(int argc, char *argv[])
{
    #ifdef TEST_JSON_DECODER

    CommsAgent comms_agent;
    WarpToWlanAgent *warp_to_wlan = new WarpToWlanAgent();
    comms_agent.set_warp_to_wlan_agent((BssidNode*)warp_to_wlan);
    comms_agent.parse_json(SAMPLE_JSON_STRING);

    #else

    CommsAgent* comms_agent;
    WarpToWlanAgent *warp_to_wlan = new WarpToWlanAgent();
    MonToWarpAgent *mon_to_warp = new MonToWarpAgent();
    DPMAgent *dpm = new DPMAgent();
    dpm->init();

    if (argc < 4)
    {
        // Comms Agent
        comms_agent = new CommsAgent();

        // Start process loop for warp to wlan
        vector<string> interfaces;
        interfaces.push_back("eth1");
        interfaces.push_back("eth1");
        thread warp_to_wlan_sniff_thread(&WarpToWlanAgent::run, warp_to_wlan, interfaces);
        warp_to_wlan_sniff_thread.detach();

        // Start process loop for mon to warp
        interfaces.clear();
        interfaces.push_back("hwsim0");
        interfaces.push_back("eth1");
        thread mon_to_warp_sniff_thread(&MonToWarpAgent::run, mon_to_warp, interfaces);
        mon_to_warp_sniff_thread.detach();


        // Start comms agent loops
        comms_agent->set_warp_to_wlan_agent((BssidNode*)warp_to_wlan);
        comms_agent->add_to_bssid_group((BssidNode*)mon_to_warp);
        comms_agent->add_to_bssid_group((BssidNode*)dpm);

        thread comms_receive_thread(&CommsAgent::recv_loop, comms_agent);
        thread comms_send_thread(&CommsAgent::send_loop, comms_agent);
        thread comms_parse_thread(&CommsAgent::parse_loop, comms_agent);
        comms_parse_thread.detach();
        comms_receive_thread.detach();
        comms_send_thread.detach();

        // Start dpm agent
        dpm->set_agent((BssidNode*)comms_agent);
        thread dpm_timed_check_thread(&DPMAgent::timed_check, dpm, 2.0);
        dpm_timed_check_thread.detach();
    }
    else
    {
        // Comms Agent
        comms_agent = new CommsAgent(argv[1], argv[2], argv[3]);

        // Start process loop for warp to wlan
        vector<string> interfaces;
        interfaces.push_back("eth1");
        interfaces.push_back("eth1");
        thread warp_to_wlan_sniff_thread(&WarpToWlanAgent::run, warp_to_wlan, interfaces);
        warp_to_wlan_sniff_thread.detach();

        // Start process loop for mon to warp
        interfaces.clear();
        interfaces.push_back("hwsim0");
        interfaces.push_back("eth1");
        thread mon_to_warp_sniff_thread(&MonToWarpAgent::run, mon_to_warp, interfaces);
        mon_to_warp_sniff_thread.detach();


        // Start comms agent loops
        comms_agent->set_warp_to_wlan_agent((BssidNode*)warp_to_wlan);
        comms_agent->add_to_bssid_group((BssidNode*)mon_to_warp);
        comms_agent->add_to_bssid_group((BssidNode*)dpm);

        thread comms_receive_thread(&CommsAgent::recv_loop, comms_agent);
        thread comms_send_thread(&CommsAgent::send_loop, comms_agent);
        thread comms_parse_thread(&CommsAgent::parse_loop, comms_agent);
        comms_parse_thread.detach();
        comms_receive_thread.detach();
        comms_send_thread.detach();

        // Start dpm agent
        dpm->set_agent((BssidNode*)comms_agent);
        thread dpm_timed_check_thread(&DPMAgent::timed_check, dpm, 2.0);
        dpm_timed_check_thread.detach();
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
