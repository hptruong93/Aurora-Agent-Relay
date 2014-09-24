/**
* warp_to_wlan_agent.h
* Date: 2014/09/19
* Author: Alan Yang
*/

#ifndef WARP_TO_WLAN_AGENT_H_
#define WARP_TO_WLAN_AGENT_H_

#include "relay_agent.h"

namespace RelayAgents {
    class WarpToWlanAgent : public RelayAgent {
        public:
        	WarpToWlanAgent();
            WarpToWlanAgent(PacketSender* init_packet_sender);
            bool process(PDU &pkt) override;
            void run(int argc, char *argv[]) override;
            static char* get_interface_name(Dot11::address_type addr);
    };   
}

#endif