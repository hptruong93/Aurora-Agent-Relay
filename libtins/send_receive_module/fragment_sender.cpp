/*
* fragment_sender.c
*
* Created on: 2014-09-02
* Author: Hoai Phuoc Truong
*/

#include <tins/tins.h>
#include <stdlib.h>
#include "fragment_sender.h"
#include "../warp_protocol/warp_protocol.h"
#include "../revised_version/util.h"
#include "../revised_version/config.h"

#include <string.h>

using namespace std;
using namespace Tins;
using namespace Config;

FragmentSender::FragmentSender(PacketSender* sender) {
    this->sender = sender;
}

FragmentSender::~FragmentSender() {
    //Do nothing
}

void FragmentSender::send(PDU& pkt, WARP_protocol::WARP_transmit_struct* transmit_info, uint8_t subtype) {
    //At this point, assume that transmit_info has been setup correctly
    EthernetII to_send = EthernetII(WARP, PC_ENGINE);
    to_send.payload_type(WARP_PROTOCOL_TYPE);

    if (subtype == SUBTYPE_MANGEMENT_TRANSMIT) {
        //We now assume that ethernet protocol is able to handle all management packets encapsulated with warp protocol header
        //-----------------> Create WARP layer and append at the end
        WARP_protocol* warp_layer = WARP_protocol::create_transmit(transmit_info, NULL, SUBTYPE_MANGEMENT_TRANSMIT);
        to_send = to_send / (*warp_layer) / (pkt);
        this->sender->send(to_send);
        cout << "Sent management" << endl;
        warp_layer->free_buffer();
        free(warp_layer);
    } else if (subtype == SUBTYPE_DATA_TRANSMIT) {
        //Some data packets need to be checked if length exceed management protocol
        WARP_protocol::WARP_fragment_struct* fragment_info = WARP_protocol::generate_fragment_struct();
        WARP_protocol* warp_layer = WARP_protocol::create_transmit(transmit_info, fragment_info, SUBTYPE_DATA_TRANSMIT);

        cout << "Total data length is " << pkt.size() + warp_layer->header_size() << endl;
        cout << "Actual data length is " << pkt.size() << endl;
        if (pkt.size() + warp_layer->header_size() > Config::MAX_ETHERNET_LENGTH) {//Split into several fragment
            
            uint16_t fragment_max_length = Config::MAX_ETHERNET_LENGTH - warp_layer->header_size();
            cout << "Too long, need to be fragmented. Available space is " << fragment_max_length << endl;

            uint8_t total_number_fragment = (pkt.size() % fragment_max_length == 0) ? (pkt.size() / fragment_max_length) : (pkt.size() / fragment_max_length + 1);
            fragment_info->total_number_fragment = total_number_fragment;

            uint8_t i = 0;
            printf("Total split %d\n", total_number_fragment);
            printf("Header size is %d\n", warp_layer->header_size());
            uint8_t* packet_data = &(pkt.serialize()[0]);

            warp_layer->free_buffer();
            free(warp_layer);
            for (; i < total_number_fragment; i++) {
                printf("Looping with index = %d\n", i);
                fragment_info->fragment_number = i;

                
                EthernetII sending = EthernetII(WARP, PC_ENGINE);
                sending.payload_type(WARP_PROTOCOL_TYPE);
                //Prepare the packet
                uint16_t byte_offset = i * (fragment_max_length);
                fragment_info->byte_offset = byte_offset;
                cout << "Offset is " << byte_offset << endl;
                uint32_t fragment_length = 1;
                if (byte_offset + fragment_max_length <= pkt.size()) {
                    fragment_length = fragment_max_length;
                } else {
                    fragment_length = pkt.size() - byte_offset;
                }
                transmit_info->payload_size = fragment_length;
                warp_layer = WARP_protocol::create_transmit(transmit_info, fragment_info, SUBTYPE_DATA_TRANSMIT);
                cout << "Fragment length is " << fragment_length << endl;

                RawPDU current_fragment(packet_data + byte_offset, fragment_length);                
                //Prepared

                //Assemble and send
                sending = sending / (*warp_layer) / current_fragment;
                this->sender->send(sending);
                warp_layer->free_buffer();
                free(warp_layer);
            }
        } else {//Send in one fragment
            to_send = to_send / (*warp_layer) / (pkt);
            cout << "Can send in one shot" << endl;
            this->sender->send(to_send);
        }

        free(fragment_info);
    }
}

int maain(void) {
    uint8_t buffer[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26};
    RawPDU aa(buffer, 27);
    PacketSender* packet_sender = new PacketSender("eth1");
    FragmentSender sender(packet_sender);

    WARP_protocol::WARP_transmit_struct transmit_info;
    transmit_info.flag = 0;
    transmit_info.retry = 7;
    transmit_info.payload_size = 5;
    uint8_t bssid[] = {65, 65, 65, 65, 65, 65};

    memcpy(transmit_info.bssid, bssid, 6);

    sender.send(aa, &transmit_info, SUBTYPE_DATA_TRANSMIT);

    return 0;
}