/*
* warp_protocol_sender.h
* Refactored from fragment_sender.h
*
* Created On: 2014-09-12
* Author: Hoai Phuoc Truong, Alan Yang
*/

#ifndef WARP_PROTOCOL_SENDER_SENDER_H_
#define WARP_PROTOCOL_SENDER_SENDER_H_

#include <tins/tins.h>
#include "../warp_protocol/warp_protocol.h"
using namespace Tins;

class WARP_ProtocolSender
{
    public:
        WARP_ProtocolSender(PacketSender* init_sender);
        ~WARP_ProtocolSender();
        void send(PDU& pkt, uint8_t type, uint8_t subtype, WARP_protocol::WARP_transmit_struct* transmit_info = NULL);
        void set_sender(PacketSender* new_sender);
        PacketSender* get_sender();
    private:
    	void init(PacketSender* init_sender);
        std::unique_ptr<PacketSender> sender;
};

#endif