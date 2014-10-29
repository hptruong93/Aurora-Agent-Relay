/**
* warp_to_wlan_agent.cpp
* Date: 2014/09/19
* Author: Alan Yang
*/

#include "warp_to_wlan_agent.h"

#include <stdio.h>
#include <string>
#include <exception>
#include <vector>
#include <time.h>

#include "../revised_version/config.h"
#include "../revised_version/util.h"
#include <tins/tins.h>
#include "../send_receive_module/warp_protocol_sender.h"
#include "../send_receive_module/fragment_receiver.h"
#include "../warp_protocol/warp_protocol.h"

using namespace Tins;
using namespace Config;
using namespace std;

using namespace RelayAgents;

WarpToWlanAgent::WarpToWlanAgent() : RelayAgent()
{
    sem_init(&this->mac_add_sync, 0, 1);
    sem_wait(&this->mac_add_sync);
    sem_init(&this->transmission_sync, 0, 1);
    sem_wait(&this->transmission_sync);
}

WarpToWlanAgent::WarpToWlanAgent(PacketSender* init_packet_sender) : RelayAgent(init_packet_sender)
{
    sem_init(&this->mac_add_sync, 0, 1);
    sem_wait(&this->mac_add_sync);
    sem_init(&this->transmission_sync, 0, 1);
    sem_wait(&this->transmission_sync);
}

bool WarpToWlanAgent::process(PDU &pkt)
{
    EthernetII &ethernet_packet = pkt.rfind_pdu<EthernetII>();
    
    if (ethernet_packet.src_addr() == Config::WARP && ethernet_packet.dst_addr() == Config::PC_ENGINE) {
        WARP_protocol &warp_layer = ethernet_packet.rfind_pdu<WARP_protocol>();
        uint8_t* warp_layer_buffer = warp_layer.get_buffer();

        WARP_protocol::WARP_transmit_struct transmit_result;
        uint32_t data_payload_length = WARP_protocol::process_warp_layer(warp_layer_buffer, &transmit_result);

        if (data_payload_length != 0) {//Has some data to forward to interface
            uint8_t* assembled_data = warp_layer_buffer + data_payload_length;
            uint32_t data_length = transmit_result.payload_size;

            //RawPDU payload(warp_layer_buffer, warp_layer.header_size());
            Dot11 dot11(assembled_data, data_length);
            auto type = dot11.type();
            if (type == Dot11::MANAGEMENT) {
                //Put in radio tap and send to output
                RadioTap header(default_radio_tap_buffer, sizeof(default_radio_tap_buffer));
                RadioTap to_send = header /  RawPDU(assembled_data, data_length);
                // char* interface_name = getInterface(management_frame.addr1());

                // if (strlen(interface_name) > 0) {
                    this->packet_sender.get()->default_interface("hwsim0");
                    this->packet_sender.get()->send(to_send);
                    cout << "Sent 1 packet to " << "hwsim0" << endl;
                // } else {
                //     cout << "ERROR: no interface found for the destination hardware address: " 
                //             << dot11.addr1().to_string() << endl;
                // }

                // free(interface_name);

            } else {
                //Drop
            }
        }
    }

    // If agent receives stop signal then this is the last process function called
    this->status_lock.lock();
    bool reutrn_code = !this->complete;
    this->status_lock.unlock();

    return reutrn_code;
}

void WarpToWlanAgent::run(vector<string> args)
{
    Allocators::register_allocator<EthernetII, Tins::WARP_protocol>(WARP_PROTOCOL_TYPE);
    int argc = args.size();
    
    if (argc >= 2) {
        this->set_in_interface(args[0].c_str());
        this->set_out_interface(args[1].c_str());
        // cout << "Init warp to wlan from " << args[0] << " to " << args[1] << endl;

        if (argc >= 3) {
            string mon_interface = args[1];
            string wlan_interface = args[2];
            cout << "Monitor interface is " << mon_interface << " and wlan interface is " << wlan_interface << endl;
        }

        if (argc == 4) {
            Config::HOSTAPD = Tins::HWAddress<6>(args[3].c_str());
            cout << "hostapd mac is " << args[3] << endl;
        }
    } else {
        this->set_in_interface("eth1");
        this->set_out_interface("mon.wlan0");

        string mon_interface = "mon.wlan0";
        string wlan_interface = "wlan0";

        cout << "Init warp to wlan from eth1 to mon.wlan0" << endl;
        cout << "Monitor interface is " << mon_interface << " and wlan interface is " << wlan_interface << endl;
    }

    initialize_receiver();
    this->sniff();
}

void WarpToWlanAgent::set_out_interface(const char* out_interface)
{
    this->out_interface.reset(new std::string(out_interface));

    this->packet_sender.reset(new PacketSender(this->out_interface.get()->c_str()));

    // NOTE: Not an ideal solution. A better solution would be changing the packet sender pointer in protocl sender class
    // into a shared_ptr
    this->protocol_sender.reset(new WARP_ProtocolSender(new PacketSender(this->out_interface.get()->c_str())));
}

int WarpToWlanAgent::timed_sync(int opertaion_code, int timeout)
{
    BSSID_NODE_OPS op = (BSSID_NODE_OPS)opertaion_code;
    struct timespec ts;
    
    if (clock_gettime(CLOCK_REALTIME, &ts) == -1)
    {
        std::cout<<"ERROR: Failed to get current time."<<std::endl;
        return -1;
    }
    ts.tv_sec += timeout;
    
    switch(op)
    {
        case BSSID_NODE_OPS::MAC_ADD:
            return sem_timedwait(&this->mac_add_sync, &ts);
        case BSSID_NODE_OPS::TRANSMISSION_CNTRL:
            return sem_timedwait(&this->transmission_sync, &ts);
    }
}

int WarpToWlanAgent::sync(int operation_code, void* data)
{
    BSSID_NODE_OPS op = (BSSID_NODE_OPS)operation_code;
    switch(op)
    {
        case BSSID_NODE_OPS::SEND_MAC_ADDR_CNTRL:
            this->protocol_sender.get()->send(*(WARP_protocol*)data, TYPE_CONTROL, SUBTYPE_MAC_ADDRESS_CONTROL);
            return 0;
        case BSSID_NODE_OPS::SEND_TRANSMISSION_CNTRL:
            this->protocol_sender.get()->send(*(WARP_protocol*)data, TYPE_CONTROL, SUBTYPE_TRANSMISSION_CONTROL);
            return 0;
    }

    return 0;
}

// Static

char* WarpToWlanAgent::get_interface_name(Dot11::address_type addr)
{
    string address = addr.to_string();
    cout << "Address is " << address << endl;

    FILE *fp;
    char *interface_name = (char*)malloc(64);
    size_t interface_name_len = 0;
    int c;
    string command = string(GREP_FROM_IFCONFIG , strlen(GREP_FROM_IFCONFIG)) + "'" + string(HW_ADDR_KEYWORD, strlen(HW_ADDR_KEYWORD)) + address + "'";
    fp = popen(command.c_str(), "r");

    while ((c = fgetc(fp)) != EOF)
    {
        if ((char) c == ' ')
        {
            break;
        }
        interface_name[interface_name_len++] = (char)c;
    }

    interface_name[interface_name_len] = '\0';

    return interface_name;
}