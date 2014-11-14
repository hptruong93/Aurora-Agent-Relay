/**
* mon_to_warp_agent.h
* Date: 2014/09/17
* Author: Alan Yang
*/
#ifndef MON_TO_WARP_AGENT_H_
#define MON_TO_WARP_AGENT_H_

#include "relay_agent.h"
#include "bssid_node.h"

#include <stdio.h>
#include <string>
#include <exception>
#include <vector>

#include "config.h"
#include "util.h"
#include <tins/tins.h>
#include "warp_protocol_sender.h"
#include "fragment_receiver.h"
#include "warp_protocol.h"

using namespace Tins;
using namespace Config;
using namespace std;

namespace RelayAgents {
    class MonToWarpAgent: public RelayAgent, public BssidNode {
        public:
        	MonToWarpAgent();
            MonToWarpAgent(WARP_ProtocolSender* init_protocol_sender);
            void set_out_interface(const char* out_interface);
            bool process(PDU &pkt);
            void run(vector<string> args);
            // override bssidnode
            int sync(int operation_code, void* addr);

            // Static methods and constants
            static bool Is_Management_Frame(int type);
        private:
    };
}

#endif