/**
* warp_to_wlan_agent.h
* Date: 2014/09/19
* Author: Alan Yang
*/

#ifndef WARP_TO_WLAN_AGENT_H_
#define WARP_TO_WLAN_AGENT_H_

#include "relay_agent.h"
#include "bssid_node.h"

#include <semaphore.h>

namespace RelayAgents {
    class WarpToWlanAgent : public RelayAgent, public BssidNode {
        public:
        	WarpToWlanAgent();
            WarpToWlanAgent(PacketSender* init_packet_sender);
            void set_out_interface(const char* out_interface);
            bool process(PDU &pkt);
            void run(vector<string> args);
            // override BssidNode
            int timed_sync(int operation_code, void* response, int timeout);
            int sync(int operation_code, void* bssid);
            static char* get_interface_name(Dot11::address_type addr);
        private:
        	sem_t mac_control_sync;
        	sem_t transmission_sync;
            uint8_t response_packet_type;
    };   
}

#endif