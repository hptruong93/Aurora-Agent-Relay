#include <thread>
#include <iostream>
#include <stdio.h>

#include <zmq.hpp>
#include <jansson.h>

#include "mon_to_warp_agent.h"
#include "wlan_to_warp_agent.h"
#include "warp_to_wlan_agent.h"
#include "comms_agent.h"
#include "agents.h"

using namespace RelayAgents;
using namespace std;

void AgentFactory::spawn_agent_thread(const char *agent_type, int argc, char *argv[])
{
    if (strcmp(agent_type, WLAN_TO_WARP) == 0)
    {
        WlanToWarpAgent agent;
        thread agent_thread(&WlanToWarpAgent::run, &agent, argc, argv);

        agent_thread.detach();
    }
}

int main(int argc, char *argv[])
{
    // Test
    WlanToWarpAgent test_wtp_agent;
    thread t1(&WlanToWarpAgent::run, &test_wtp_agent, argc, argv);

    t1.join();

    return 0;
}