#include "mon_to_warp_agent.h"
#include "wlan_to_warp_agent.h"
#include "warp_to_wlan_agent.h"

#include <iostream>

#include <stdio.h>
#include <string>
#include <exception>

#include "config.h"
#include "util.h"
#include <tins/tins.h>
#include "../send_receive_module/warp_protocol_sender.h"
#include "../send_receive_module/fragment_receiver.h"
#include "../warp_protocol/warp_protocol.h"

using namespace Tins;
using namespace Config;
using namespace std;

using namespace RelayAgents;

int main(int argc, char *argv[])
{
	// Test
	WlanToWarpAgent test_wtp_agent;
	test_wtp_agent.run(argc, argv);
}