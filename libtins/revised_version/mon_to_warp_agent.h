/**
* mon_to_warp_agent.h
* Date: 2014/09/17
* Author: Alan Yang
*/

#include "relay_agent.h"

#ifndef MON_TO_WARP_AGENT_H_
#define MON_TO_WARP_AGENT_H_

class MonToWarpAgent: public RelayAgent {
    public:
        MonToWarpAgent(WARP_ProtocolSender* init_protocol_sender);
        bool process(PDU &pkt) override;
        void run(int argc, char *argv[]) override;

        // Static methods and constants
        static bool Is_Management_Frame(int type);
};

#endif