/*
* fragment_sender.h
*
* Created on: 2014-09-02
* Author: Hoai Phuoc Truong
*/
#ifndef FRAGMENT_SENDER_H_
#define FRAGMENT_SENDER_H_

#include <tins/tins.h>
#include "../warp_protocol/warp_protocol.h"
using namespace Tins;

class FragmentSender {

public:
    FragmentSender(PacketSender* init_sender);
    ~FragmentSender();

    void send(PDU& pkt, uint8_t type, uint8_t subtype, WARP_protocol::WARP_transmit_struct* transmit_info);

    void set_sender(PacketSender* new_sender) {
        this->sender = new_sender;
    }

    PacketSender* get_sender() {
        return this->sender;
    }

private:
    PacketSender* sender;
};

#endif