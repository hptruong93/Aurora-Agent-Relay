/*
* warp_protocol_sender.cpp
* Refactroed from fragment_sender.cpp
*
* Created On: 2014-09-12
* Author: Hoai Phuoc Truong, Alan Yang
*/

#include <stdlib.h>
#include "warp_protocol_sender.h"
#include "../warp_protocol/warp_protocol.h"
#include "../revised_version/util.h"
#include "../revised_version/config.h"

#include <string.h>

using namespace std;
using namespace Tins;
using namespace Config;

WARP_ProtocolSender::WARP_ProtocolSender(PacketSender* init_sender)
{
    this->sender.reset(init_sender);
}

WARP_ProtocolSender::~WARP_ProtocolSender()
{
    this->sender.reset();
}

void WARP_ProtocolSender::send(PDU& pkt, uint8_t type, uint8_t subtype, WARP_protocol::WARP_transmit_struct* transmit_info)
{
    PacketSender* sender = this->get_sender();

    //At this point, assume that transmit_info has been setup correctly
    EthernetII to_send = EthernetII(WARP, PC_ENGINE);
    to_send.payload_type(WARP_PROTOCOL_TYPE);

    if (type == TYPE_TRANSMIT) {
        // Transmit Info cannot be null
        if (transmit_info == NULL) {
            cout<<"Transmit Info cannot be null for transmit packet."<<endl;
            return;
        }

        //Some data packets need to be checked if length exceed ethernet protocol
        WARP_protocol::WARP_fragment_struct* fragment_info = WARP_protocol::generate_fragment_struct();
        WARP_protocol* init_warp_layer = WARP_protocol::create_transmit(transmit_info, fragment_info, subtype);

        // cout << "Total data length is " << pkt.size() + warp_layer->header_size() << endl;
        // cout << "Actual data length is " << pkt.size() << endl;
        if (pkt.size() + init_warp_layer->header_size() > Config::MAX_ETHERNET_LENGTH) {//Split into several fragment
            uint16_t fragment_max_length = Config::MAX_ETHERNET_LENGTH - init_warp_layer->header_size();
            // cout << "Too long, need to be fragmented. Available space is " << fragment_max_length << endl;

            uint8_t total_number_fragment = (pkt.size() % fragment_max_length == 0) ? (pkt.size() / fragment_max_length) : (pkt.size() / fragment_max_length + 1);
            fragment_info->total_number_fragment = total_number_fragment;

            uint8_t i = 0;
            // printf("Total split %d\n", total_number_fragment);
            // printf("Header size is %d\n", warp_layer->header_size());
            std::vector<uint8_t> packet_data = pkt.serialize();

            //For some reason can't use the packet_data as the buffer....????!!!!
            static uint8_t temp_buffer[2048];
            memcpy(temp_buffer, &(packet_data[0]), packet_data.size());

            // printf("First at %d\n", temp_buffer);
            // uint8_t j = 0;
            // for (; j < 45; j++) {
            //     printf("%02x-", *(temp_buffer + 0 + j));
            // }
            // printf("\n");

            for (; i < total_number_fragment; i++) {
                // printf("Looping with index = %d\n", i);
                fragment_info->fragment_number = i;

                
                EthernetII sending = EthernetII(WARP, PC_ENGINE);
                sending.payload_type(WARP_PROTOCOL_TYPE);

                //Prepare the packet
                uint16_t byte_offset = i * (fragment_max_length);
                fragment_info->byte_offset = byte_offset;
                //cout << "Offset is " << byte_offset << endl;
                uint32_t fragment_length = 1;
                if (byte_offset + fragment_max_length <= pkt.size()) {
                    fragment_length = fragment_max_length;
                } else {
                    fragment_length = pkt.size() - byte_offset;
                }
                transmit_info->payload_size = fragment_length;

                // printf("Just before %d at %d\n", 0, temp_buffer);
                // for (j = 0; j < 45; j++) {
                //     printf("%02x-", *(temp_buffer + 0 + j));
                // }
                // printf("\n");

                WARP_protocol* warp_layer = WARP_protocol::create_transmit(transmit_info, fragment_info, subtype);
                // cout << "Fragment length is " << fragment_length << endl;

                // printf("Offset %d at %d\n", byte_offset, temp_buffer);
                // for (j = 0; j < 45; j++) {
                //     printf("%02x-", *(temp_buffer + byte_offset + j));
                // }
                // printf("\n");

                RawPDU current_fragment(temp_buffer + byte_offset, fragment_length);                
                //Prepared

                //Assemble and send
                sending = sending / (*warp_layer) / current_fragment;

                sender->send(sending);
                warp_layer->free_buffer();
                delete warp_layer;
            }
        } else {//Send in one fragment
            // printf("Total split %d\n", total_number_fragment);
            // printf("Header size is %d\n", warp_layer->header_size());
            // uint8_t* packet_data = &(pkt.serialize()[0]);

            // uint8_t j = 0;
            // for (; j < 20; j++) {
            //     printf("%02x-", *(packet_data + 0 + j));
            // }
            // printf("\n");

            to_send = to_send / (*init_warp_layer) / (pkt);
            //cout << "Can send in one shot" << endl;
            sender->send(to_send);
        }

        init_warp_layer->free_buffer();
        delete init_warp_layer;
        free(fragment_info);
    } else if (type == TYPE_CONTROL) {
        //The packet passed in with the input is the control packet. Append this to the ethernet header and send
        //It is safe to assume that the WARP protocol length will never exceed ethernet payload length
        to_send = to_send / pkt;
        sender->send(to_send);
        cout << "Sent one control packet" << endl;
    }
}

void WARP_ProtocolSender::set_sender(PacketSender* new_sender)
{
    this->sender.reset(new_sender);
}

PacketSender* WARP_ProtocolSender::get_sender()
{
    return this->sender.get();
}

// For testing
int maain(void) {
    uint8_t buffer[] = {7, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26};
    RawPDU aa(buffer, sizeof(buffer));

    PacketSender* packet_sender = new PacketSender("eth1");
    WARP_ProtocolSender sender(packet_sender);

    WARP_protocol::WARP_transmit_struct transmit_info;
    transmit_info.flag = 0;
    transmit_info.retry = 7;
    transmit_info.payload_size = sizeof(buffer);
    cout << "length is " << sizeof(buffer) + 5 << endl;
    uint8_t bssid[] = {65, 65, 65, 65, 65, 65};

    memcpy(transmit_info.bssid, bssid, 6);

    sender.send(aa, TYPE_CONTROL, SUBTYPE_MAC_ADDRESS_CONTROL);

    /*
     * Uncomment this to test sending control packets
    WARP_protocol::WARP_mac_control_struct* mac_control;
    mac_control = (WARP_protocol::WARP_mac_control_struct*) calloc(sizeof(WARP_protocol::WARP_mac_control_struct), sizeof(uint8_t));
    mac_control->operation_code = 0;
    uint8_t mac_address[] = {10, 10, 10, 10, 10, 9};
    memcpy(mac_control->mac_address, mac_address, 6);
    WARP_protocol* aa = WARP_protocol::create_mac_control(mac_control);
    sender.send((*aa), TYPE_CONTROL, SUBTYPE_MAC_ADDRESS_CONTROL);
    */

    return 0;
}