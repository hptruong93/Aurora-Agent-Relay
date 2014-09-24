#include "mon_to_warp_agent.h"
#include "wlan_to_warp_agent.h"
#include "warp_to_wlan_agent.h"
#include <thread>


#include <iostream>

using namespace RelayAgents;
using namespace std;

int main(int argc, char *argv[])
{
	// Test
	WlanToWarpAgent test_wtp_agent;
	thread t1(&WlanToWarpAgent::run, &test_wtp_agent, argc, argv);

	t1.join();

	return 0;
}