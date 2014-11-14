
#include <stdio.h>
#include <string.h>
#include <exception>

#include <tins/tins.h>
#include "config.h"
#include "../warp_protocol/warp_protocol.h"
#include "../send_receive_module/fragment_receiver.h"
#include "util.h"

using namespace Tins;
using namespace std;
using namespace Config;

PacketSender *sender;
string in_interface;

string mon_interface;
string wlan_interface;

string PDUTypeToString(int PDUTypeFlag) {
    string definitions[] = {
        "RAW", "ETHERNET_II", "IEEE802_3", "RADIOTAP",
        "DOT11", "DOT11_ACK", "DOT11_ASSOC_REQ", "DOT11_ASSOC_RESP",
        "DOT11_AUTH", "DOT11_BEACON", "DOT11_BLOCK_ACK", "DOT11_BLOCK_ACK_REQ",
        "DOT11_CF_END", "DOT11_DATA", "DOT11_CONTROL", "DOT11_DEAUTH",
        "DOT11_DIASSOC", "DOT11_END_CF_ACK", "DOT11_MANAGEMENT", "DOT11_PROBE_REQ",
        "DOT11_PROBE_RESP", "DOT11_PS_POLL", "DOT11_REASSOC_REQ", "DOT11_REASSOC_RESP",
        "DOT11_RTS", "DOT11_QOS_DATA", "LLC", "SNAP",
        "IP", "ARP", "TCP", "UDP",
        "ICMP", "BOOTP", "DHCP", "EAPOL",
        "RC4EAPOL", "RSNEAPOL", "DNS", "LOOPBACK",
        "IPv6", "ICMPv6", "SLL", "DHCPv6",
        "DOT1Q", "PPPOE", "STP", "PPI",
        "IPSEC_AH", "IPSEC_ESP"
    };
    if(PDUTypeFlag == 1000)
        return string("USER_DEFINED_PDU");
    else
        return definitions[PDUTypeFlag];
}

bool process(PDU &pkt) {
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
                    sender->default_interface(mon_interface);
                    sender->send(to_send);
                    cout << "Sent 1 packet to " << mon_interface << endl;
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
    return true;
}

void sniff(string in_interface) {
    Sniffer sniffer(in_interface, Sniffer::PROMISC);
    sniffer.sniff_loop(process);
}

void set_in_interface(const char* input) {
    string temp(input);
    in_interface = temp;
}

void set_out_interface(const char* output) {
    delete sender;
    sender = new PacketSender(output);
}


int main(int argc, char *argv[]) {
    Allocators::register_allocator<EthernetII, Tins::WARP_protocol>(WARP_PROTOCOL_TYPE);
    
    if (argc >= 3) {
        set_in_interface(argv[1]);
        set_out_interface(argv[2]);
        cout << "Init warp to wlan from " << argv[1] << " to " << argv[2] << endl;

        if (argc >= 4) {
            mon_interface = argv[2];
            wlan_interface = argv[3];
            cout << "Monitor interface is " << mon_interface << " and wlan interface is " << wlan_interface << endl;
        }

        if (argc == 5) {
            Config::HOSTAPD = Tins::HWAddress<6>(argv[4]);
            cout << "hostapd mac is " << argv[4] << endl;
        }
    } else {
        set_in_interface("eth1");
        set_out_interface("mon.wlan0");

        mon_interface = "mon.wlan0";
        wlan_interface = "wlan0";

        cout << "Init warp to wlan from eth1 to mon.wlan0" << endl;
        cout << "Monitor interface is " << mon_interface << " and wlan interface is " << wlan_interface << endl;
    }

    initialize_receiver();
    sniff(in_interface);
}
