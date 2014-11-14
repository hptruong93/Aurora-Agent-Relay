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

void WARP_ProtocolSender::init(PacketSender* init_sender)
{
    this->sender = nullptr;

    if (init_sender != NULL) {
        this->sender.reset(init_sender);
    }
}

WARP_ProtocolSender::WARP_ProtocolSender(PacketSender* init_sender)
{
    this->init(init_sender);
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
    cout << "Source is " << PC_ENGINE << endl;
    cout << "Dest is " << WARP << endl;
    to_send.payload_type(WARP_PROTOCOL_TYPE);

    if (type == TYPE_TRANSMIT) {
        // Transmit Info cannot be null
        if (transmit_info == NULL) {
            cout<< "Transmit Info cannot be null for transmit packet." << endl;
            return;
        }

        //Some data packets need to be checked if length exceed ethernet protocol
        //For now assume all packets are within ethernet length
        WARP_protocol* init_warp_layer = WARP_protocol::create_transmit(transmit_info, subtype);

        to_send = to_send / (*init_warp_layer) / (pkt);
        sender->send(to_send);

        delete init_warp_layer;
        cout << "Sent 1 transmit packet" << endl;
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