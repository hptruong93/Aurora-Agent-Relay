
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

char* getInterface(Dot11::address_type addr) {
    // string address = addr.to_string();
    // FILE *fp;
    // char *interface_name = (char*)malloc(64);
    // size_t interface_name_len = 0;
    // int c;
    // string command = string(GREP_FROM_IFCONFIG , strlen(GREP_FROM_IFCONFIG)) + "'" + string(HW_ADDR_KEYWORD, strlen(HW_ADDR_KEYWORD)) + address + "'";
    // fp = popen(command.c_str(), "r");

    // while ((c = fgetc(fp)) != EOF)
    // {
    //     if ((char) c == ' ')
    //     {
    //         break;
    //     }
    //     interface_name[interface_name_len++] = (char)c;
    // }

    // interface_name[interface_name_len] = '\0';

    // return interface_name;
    return "wlan0";
}

bool process(PDU &pkt) {
    EthernetII &ethernet_packet = pkt.rfind_pdu<EthernetII>();
    
    if (ethernet_packet.src_addr() == Config::WARP && ethernet_packet.dst_addr() == Config::PC_ENGINE) {
        WARP_protocol &warp_layer = ethernet_packet.rfind_pdu<WARP_protocol>();
        uint8_t* warp_layer_buffer = warp_layer.get_buffer();

        WARP_protocol::WARP_transmit_struct transmit_result;
        uint32_t fragment_index = WARP_protocol::process_warp_layer(warp_layer_buffer, &transmit_result);

        receive_result receive_result;
        packet_receive(warp_layer_buffer + fragment_index, transmit_result.payload_size, &receive_result);

        if (receive_result.status == READY_TO_SEND) {
            uint8_t* assembled_data = receive_result.packet_address;
            uint32_t data_length = receive_result.info_address->length;

            //RawPDU payload(warp_layer_buffer, warp_layer.header_size());
            Dot11 dot11(assembled_data, data_length);
            auto type = dot11.type();
            if (type == Dot11::MANAGEMENT) {
                //Put in radio tap and send to output
                RadioTap header(default_radio_tap_buffer, sizeof(default_radio_tap_buffer));
                RadioTap to_send = header /  RawPDU(assembled_data, data_length);
                // char* interface_name = getInterface(management_frame.addr3());

                // if (strlen(interface_name) > 0) {
                    sender->default_interface(mon_interface);
                    sender->send(to_send);
                    cout << "Sent 1 packet to " << mon_interface << endl;
                // } else {
                //     cout << "ERROR: no interface found for the destination hardware address: " 
                //             << dot11.addr1().to_string() << endl;
                // }

                // free(interface_name);

            } else if (type == Dot11::DATA) {
                if (data_length <= 46) {//Magic?? 32 is the length of 802.11 header. 46 is min length of ethernet payload
                    //Padd with 0??? WHY CAN'T I PADD THIS?
                    // uint8_t i;
                    // for (i = data_length; i < 46; i++) {
                    //     assembled_data[i] = 0;
                    // }
                    // data_length = 46;
                    return true;
                }

                Dot11Data data_frame(assembled_data, data_length);
                
                if (data_frame.inner_pdu()->pdu_type() == PDU::RAW) {
                    RadioTap header(default_radio_tap_buffer, sizeof(default_radio_tap_buffer));
                    RadioTap to_send = header /  RawPDU(assembled_data, data_length);

                    char* interface_name = getInterface(data_frame.addr3());

                    if (strlen(interface_name) > 0) {
                        sender->default_interface(wlan_interface);
                        sender->send(to_send);
                        cout << "Sent 1 packet to " << wlan_interface << endl;
                    } else {
                        cout << "ERROR: no interface found for the destination hardware address: " 
                                << data_frame.addr3().to_string() << endl;
                    }

                    // free(interface_name);

                } else {
                    try {
                        SNAP snap = data_frame.rfind_pdu<SNAP>();

                        //Append ethernet frame then send. But from where and to where???
                        EthernetII to_send = EthernetII(data_frame.addr3(), data_frame.addr2());
                        to_send = to_send / (*(snap.inner_pdu()));
                        to_send.payload_type(snap.eth_type());

                        char* interface_name = getInterface(data_frame.addr3());

                        if (strlen(interface_name) > 0) {
                            sender->default_interface(interface_name);
                            sender->send(to_send);
                            cout << "Sent 1 packet to " << interface_name << endl;
                        } else {
                            cout << "ERROR: no interface found for the destination hardware address: " 
                                    << data_frame.addr3().to_string() << endl;
                        }

                        // free(interface_name);
                    } catch (exception& e) {
                        cout << "Snap not found. Not raw either. Payload is of type " << PDUTypeToString(data_frame.inner_pdu()->pdu_type()) << endl;
                    }
                }
            } else if (type == Dot11::CONTROL) {
                cout << "Drop control packet." << endl;    
            } else {
                cout << "Invalid IEEE802.11 packet type..." << endl;
            }
        }

        free(receive_result.info_address);
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
