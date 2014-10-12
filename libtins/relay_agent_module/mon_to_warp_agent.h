/**
* mon_to_warp_agent.h
* Date: 2014/09/17
* Author: Alan Yang
*/

#include "relay_agent.h"

using namespace Tins;
using namespace Config;
using namespace std;

#ifndef MON_TO_WARP_AGENT_H_
#define MON_TO_WARP_AGENT_H_

namespace RelayAgents {
    class MonToWarpAgent: public RelayAgent {
        public:
        	MonToWarpAgent();
            MonToWarpAgent(WARP_ProtocolSender* init_protocol_sender);
            void set_out_interface(const char* out_interface);
            bool process(PDU &pkt);
            void run(vector<string> args);

            // Static methods and constants
            static bool Is_Management_Frame(int type);
    };
}

#endif