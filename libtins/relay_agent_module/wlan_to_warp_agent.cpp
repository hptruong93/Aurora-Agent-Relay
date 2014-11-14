/**
* wlan_to_warp_agent.cpp
* Date: 2014/09/19
* Author: Alan Yang
*/

#include "wlan_to_warp_agent.h"

#include <stdio.h>
#include <string>
#include <exception>
#include <vector>
#include <iostream>

#include "config.h"
#include "util.h"
#include <tins/tins.h>
#include "warp_protocol_sender.h"
#include "fragment_receiver.h"
#include "warp_protocol.h"

using namespace Tins;
using namespace Config;
using namespace std;

using namespace RelayAgents;

WlanToWarpAgent::WlanToWarpAgent() : RelayAgent()
{
    
}

WlanToWarpAgent::WlanToWarpAgent(WARP_ProtocolSender* init_protocol_sender) : RelayAgent(init_protocol_sender)
{

}

bool WlanToWarpAgent::process(PDU &pkt)
{
    WARP_ProtocolSender* protocol_sender = this->protocol_sender.get();

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
            protocol_sender->send(pkt, TYPE_TRANSMIT, SUBTYPE_DATA_TRANSMIT, transmit_info);

            //-----------------> Clean up
            free(transmit_info);
            cout << "Sent 1 data packet" << endl;
        }
    } else {
        cerr << "Error: Non EthernetII Packet Detected!" << endl;
    }

    // If agent receives stop signal then this is the last process function called
    this->status_lock.lock();
    bool reutrn_code = !this->complete;
    this->status_lock.unlock();

    return reutrn_code;
}

void WlanToWarpAgent::run(vector<string> args)
{
    cout<<"Child!"<<endl;
    int argc = args.size();

    std::cout<<argc<<std::endl;

    if (argc >= 2) {
        this->set_in_interface(args[0].c_str());
        this->set_out_interface(args[1].c_str());
        cout << "Init pc to warp from " << args[0] << " to " << args[1] << endl;

        if (argc == 3) {
            Config::HOSTAPD = Tins::HWAddress<6>(args[2].c_str());
            cout << "hostapd mac is " << args[2] << endl;
        }        
    } else {
        this->set_in_interface("hwsim0");
        this->set_out_interface("eth0");
        cout << "Init pc to warp from hwsim0 to eth0" << endl;
    }

    this->sniff();
}

void WlanToWarpAgent::set_out_interface(const char* out_interface)
{
    this->out_interface.reset(new std::string(out_interface));

    this->protocol_sender.reset(new WARP_ProtocolSender(new PacketSender(this->out_interface.get()->c_str())));
}