/*
* fragment_manager.h
*
* Created on: 2014-09-02
* Author: Hoai Phuoc Truong & Alfred Kenny
*/
#include <tins/tins.h>
#include "../warp_protocol/warp_protocol.h"

#ifndef FRAGMENT_RECEIVER_H_
#define FRAGMENT_RECEIVER_H_

#define READY_TO_SEND                           1
#define WAITING_FOR_FIRST_FRAGMENT              2
#define WAITING_FOR_FRAGMENT                    3

typedef struct {
    uint8_t status;
    uint8_t* packet_address;
    Tins::WARP_protocol::WARP_fragment_struct* info_address;
} receive_result;

//The buffer will starts with fragment info. WARP should not distinguish between management and data packets
void packet_receive(uint8_t* packet_buffer, uint32_t data_length, receive_result* result);
#endif