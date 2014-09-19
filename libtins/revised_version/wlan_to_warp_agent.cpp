/**
* wlan_to_warp_agent.cpp
* Date: 2014/09/19
* Author: Alan Yang
*/

#include "wlan_to_warp_agent.h"

WlanToWarpAgent::WlanToWarpAgent(WARP_ProtocolSender* init_protocol_sender) : RelayAgent(init_protocol_sender)
{

}

bool WlanToWarpAgent::process(PDU &pkt) override
{
    if (pkt.pdu_type() == pkt.ETHERNET_II || pkt.pdu_type() == pkt.IEEE802_3) {
        //For now forward everything
        if (1 == 1) {
            //Add an ethernet frame and send over iface to warp
            //-----------------> Create WARP transmit info
            WARP_protocol::WARP_transmit_struct* transmit_info = WARP_protocol::get_default_transmit_struct();
            transmit_info->retry = MAX_RETRY;
            transmit_info->payload_size = (uint16_t) pkt.size();

            //-----------------> Create WARP layer and append at the end
            cout << "Original ethernet packet size is " << pkt.size() << endl;
            this->protocol_sender->send(pkt, TYPE_TRANSMIT, SUBTYPE_DATA_TRANSMIT, transmit_info);

            //-----------------> Clean up
            free(transmit_info);
            cout << "Sent 1 data packet" << endl;
        }
    } else {
        cerr << "Error: Non EthernetII Packet Detected!" << endl;
    }
    return true;
}

void WlanToWarpAgent(int argc, char *argv[]) override
{
    if (argc >= 3) {
        this->set_in_interface(argv[1]);
        this->set_out_interface(argv[2]);
        cout << "Init pc to warp from " << argv[1] << " to " << argv[2] << endl;

        if (argc == 4) {
            Config::HOSTAPD = Tins::HWAddress<6>(argv[3]);
            cout << "hostapd mac is " << argv[3] << endl;
        }        
    } else {
        this->set_in_interface("hwsim0");
        this->set_out_interface("eth0");
        cout << "Init pc to warp from hwsim0 to eth0" << endl;
    }

    this->sniff();
}