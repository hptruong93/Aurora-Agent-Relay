#include "relay_agent.h"
#include "mon_to_warp_agent.h"
#include "wlan_to_warp_agent.h"
#include "warp_to_wlan_agent.h"

#include <iostream>

using namespace RelayAgents;

int main(int argc, char *argv[])
{
	// Test
	WlanToWarpAgent test_wtp_agent;
	test_wtp_agent.run(argc, argv);
}