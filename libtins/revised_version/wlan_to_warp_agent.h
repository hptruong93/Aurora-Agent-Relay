/**
* wlan_to_warp_agent.h
* Date: 2014/09/19
* Author: Alan Yang
*/

#ifndef WLAN_TO_WARP_AGENT_H_
#define WLAN_TO_WARP_AGENT_H_

#include "relay_agent.h"

namespace RelayAgents {
    class WlanToWarpAgent : public RelayAgent {
        public:
            WlanToWarpAgent(WARP_ProtocolSender* init_protocol_sender);
            bool process(PDU &pkt) override;
            void run(int argc, char *argv[]) override;
    };
}

#endif