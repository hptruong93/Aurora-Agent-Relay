/**
* relay_agent.h
* Date: 2014/09/17
* Author: Alan Yang
*/
#ifndef RELAYAGENT_H_
#define RELAYAGENT_H_

#include <stdio.h>
#include <string>

#include "config.h"
#include "util.h"
#include <tins/tins.h>
#include "../send_receive_module/warp_protocol_sender.h"
#include "../warp_protocol/warp_protocol.h"

using namespace Tins;
using namespace Config;
using namespace std;

class RelayAgent {
    public:
        RelayAgent(WARP_ProtocolSender* init_protocol_sender);
        ~RelayAgent();
        void sniff();
        void set_in_interface(const char* set_in_interface);
        void set_out_interface(const char* out_interface);
        WARP_ProtocolSender* getSender() const;
        virtual bool process(PDU &pkt);
        virtual void run(int argc, char **argv);
    private:
        std::unique_ptr<WARP_ProtocolSender> protocol_sender;
        std::unique_ptr<std::string> in_interface;
        std::unique_ptr<std::string> out_interface;
};

#endif