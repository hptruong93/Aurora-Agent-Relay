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

#include "config.h"
#include "util.h"
#include <tins/tins.h>
#include "warp_protocol_sender.h"
#include "fragment_receiver.h"
#include "warp_protocol.h"
#include "packet_filter.h"

#include "test.h"

using namespace Tins;
using namespace Config;
using namespace std;

using namespace RelayAgents;


#define INFO fprintf(stdout, "info: %s:%d: ", __FILE__, __LINE__)


WarpToWlanAgent::WarpToWlanAgent() : RelayAgent()
{
    sem_init(&this->mac_control_sync, 0, 1);
    sem_wait(&this->mac_control_sync);
    sem_init(&this->transmission_sync, 0, 1);
    sem_wait(&this->transmission_sync);
    sem_init(&this->bssid_control_sync, 0, 1);
    sem_wait(&this->bssid_control_sync);
}

WarpToWlanAgent::WarpToWlanAgent(PacketSender* init_packet_sender) : RelayAgent(init_packet_sender)
{
    sem_init(&this->mac_control_sync, 0, 1);
    sem_wait(&this->mac_control_sync);
    sem_init(&this->transmission_sync, 0, 1);
    sem_wait(&this->transmission_sync);
    sem_init(&this->bssid_control_sync, 0, 1);
    sem_wait(&this->bssid_control_sync);
}

bool WarpToWlanAgent::process(PDU &pkt)
{
    EthernetII &ethernet_packet = pkt.rfind_pdu<EthernetII>();
    
    if (ethernet_packet.src_addr() == Config::WARP && ethernet_packet.dst_addr() == Config::PC_ENGINE) {
        WARP_protocol &warp_layer = ethernet_packet.rfind_pdu<WARP_protocol>();
        uint8_t* warp_layer_buffer = warp_layer.get_buffer();

        // TODO: parse bssid control renponse

        WARP_protocol::WARP_transmit_struct transmit_result;
        WARP_protocol::WARP_transmission_control_struct transmission_result;
        WARP_protocol::WARP_mac_control_struct mac_result;
        WARP_protocol::WARP_bssid_control_struct bssid_result;
        uint8_t packet_type = WARP_protocol::check_warp_layer_type(warp_layer_buffer);
        uint32_t data_processed_length = 0;
        
        //printf("Type is %d\n", packet_type);

        switch(packet_type)
        {
            // Need to modify type parsing because subtype and type might share a same ID
            case TYPE_TRANSMIT:
                data_processed_length = WARP_protocol::process_warp_layer(warp_layer_buffer, &transmit_result);
                break;
            case SUBTYPE_TRANSMISSION_CONTROL:
                data_processed_length = WARP_protocol::process_warp_layer(warp_layer_buffer, &transmission_result);
                break;
            case SUBTYPE_MAC_ADDRESS_CONTROL:
                cout << "Received Mac Control Packet from WARP" << endl;
                data_processed_length = WARP_protocol::process_warp_layer(warp_layer_buffer, &mac_result);
                break;
            case SUBTYPE_BSSID_CONTROL:
                cout << "Bssid control packet response from WARP" << endl;
                data_processed_length = WARP_protocol::process_warp_layer(warp_layer_buffer, &bssid_result);

        }

        //cout << "Data processed length: " << data_processed_length << endl;

        if (data_processed_length != 0) {//Has some data to forward to interface
            if (packet_type == TYPE_TRANSMIT) {
                uint8_t* assembled_data = warp_layer_buffer + data_processed_length;
                uint32_t data_length = transmit_result.payload_size;

                //RawPDU payload(warp_layer_buffer, warp_layer.header_size());
                Dot11 dot11(assembled_data, data_length);
                auto type = dot11.type();
                if (type == Dot11::MANAGEMENT) {
                    //Put in radio tap and send to output
                    RadioTap header(default_radio_tap_buffer, sizeof(default_radio_tap_buffer));
                    RadioTap to_send = header /  RawPDU(assembled_data, data_length);

                    char* interface_name = get_interface(dot11.addr1());

                    // Length 4 for prefix "mon."
                    if (strlen(interface_name) > 4) {
                        string interfaces(interface_name);
                        vector<string> names;
                        split_string(interfaces, "\n", &names);

                        for (int i = 0; i < names.size(); i++)
                        {
                            this->packet_sender.get()->default_interface(names[i].c_str());
                            this->packet_sender.get()->send(to_send);
                            // cout << "Sent 1 packet to " << names[i] << endl;
                        }
                    } else {
                        cout << "ERROR: no interface found for the destination hardware address: " 
                                << dot11.addr1().to_string() << endl;
                    }

                    free(interface_name);
                }
            } else if (packet_type == SUBTYPE_TRANSMISSION_CONTROL) {
                this->response_packet_type = transmission_result.operation_code;
                sem_post(&this->transmission_sync);
            } else if (packet_type == SUBTYPE_MAC_ADDRESS_CONTROL) {
                this->response_packet_type = mac_result.operation_code;
                sem_post(&this->mac_control_sync);
            } else if (packet_type == SUBTYPE_BSSID_CONTROL) {
                this->response_packet_type = bssid_result.operation_code;
                printf("Bssid response op code: %d\n", this->response_packet_type);
                sem_post(&this->bssid_control_sync);
            }

        } else {
                //Drop
        }
    }

    // If agent receives stop signal then this is the last process function called
    this->status_lock.lock();
    bool return_code = !this->complete;
    this->status_lock.unlock();

    return return_code;
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
        this->set_out_interface("eth1");

        // string mon_interface = "mon.wlan0";
        // string wlan_interface = "wlan0";

        cout << "Init warp to wlan from eth1 to mon.wlan0\n";
        // cout << "Monitor interface is " << mon_interface << " and wlan interface is " << wlan_interface << endl;
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

int WarpToWlanAgent::timed_sync(int operation_code, void* response, int timeout)
{
    #ifndef TEST_JSON_DECODER
    BSSID_NODE_OPS op = (BSSID_NODE_OPS)operation_code;
    struct timespec ts;
    
    if (clock_gettime(CLOCK_REALTIME, &ts) == -1)
    {
        std::cout<<"ERROR: Failed to get current time."<<std::endl;
        return -1;
    }
    ts.tv_sec += timeout;

    int return_code = 0;
    
    switch(op)
    {
        case BSSID_NODE_OPS::MAC_ADD:
        case BSSID_NODE_OPS::MAC_REMOVE:
            return_code =  sem_timedwait(&this->mac_control_sync, &ts);
            *(uint8_t*)response = this->response_packet_type;
            return return_code;
        case BSSID_NODE_OPS::TRANSMISSION_CNTRL:
            return_code = sem_timedwait(&this->transmission_sync, &ts);
            *(uint8_t*)response = this->response_packet_type;
            return return_code;
        case BSSID_NODE_OPS::BSSID_CNTRL:
            return_code = sem_timedwait(&this->bssid_control_sync, &ts);
            *(uint8_t*)response = this->response_packet_type;
            return return_code;
    }
    return return_code;
    #else
    BSSID_NODE_OPS op = (BSSID_NODE_OPS)operation_code;
    
    switch(op)
    {
        case BSSID_NODE_OPS::MAC_ADD:
            cout<<"Simulation: Waiting for WARP response for mac add..."<<endl;
            return 0;
        case BSSID_NODE_OPS::TRANSMISSION_CNTRL:
            cout<<"Simulation: Waiting for WARP response for transmission control..."<<endl;
            return 0;
    }
    return 0;
    #endif
}

int WarpToWlanAgent::sync(int operation_code, void* data)
{
    #ifndef TEST_JSON_DECODER
    BSSID_NODE_OPS op = (BSSID_NODE_OPS)operation_code;
    switch(op)
    {
        case BSSID_NODE_OPS::SEND_MAC_ADDR_CNTRL:
            this->protocol_sender.get()->send(*(WARP_protocol*)data, TYPE_CONTROL, SUBTYPE_MAC_ADDRESS_CONTROL);
            return 0;
        case BSSID_NODE_OPS::SEND_TRANSMISSION_CNTRL:
            this->protocol_sender.get()->send(*(WARP_protocol*)data, TYPE_CONTROL, SUBTYPE_TRANSMISSION_CONTROL);
            return 0;
        case BSSID_NODE_OPS::SEND_BSSID_CNTRL:
            this->protocol_sender.get()->send(*(WARP_protocol*)data, TYPE_CONTROL, SUBTYPE_BSSID_CONTROL);
            return 0;
    }

    return 0;
    #else
    BSSID_NODE_OPS op = (BSSID_NODE_OPS)operation_code;
    switch(op)
    {
        case BSSID_NODE_OPS::SEND_MAC_ADDR_CNTRL:
            cout<<"Simulation: Sending mac addr control packet..."<<endl;
            return 0;
        case BSSID_NODE_OPS::SEND_TRANSMISSION_CNTRL:
            cout<<"Simulation: Sending transmission control packet..."<<endl;
            return 0;
    }
    return 0;
    #endif
}