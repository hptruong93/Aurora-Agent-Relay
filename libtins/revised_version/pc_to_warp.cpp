#include <stdio.h>
#include <string>

#include "config.h"
#include <tins/tins.h>


using namespace Tins;
using namespace Config;
using namespace std;

PacketSender *sender;
string in_interface;

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

bool is_management_frame(int type) {
	string converted = PDUTypeToString(type);
	if (converted == "DOT11" ||
        converted == "DOT11_BEACON" ||
		converted == "DOT11_PROBE_REQ" ||
		converted == "DOT11_PROBE_RESP" ||
		converted == "DOT11_AUTH" ||
		converted == "DOT11_DEAUTH" ||
		converted == "DOT11_ASSOC_REQ" ||
		converted == "DOT11_ASSOC_RESP" ||
		converted == "DOT11_REASSOC_REQ" ||
		converted == "DOT11_REASSOC_RESP" ||
		converted == "DOT11_DIASSOC") {
		return true;
	} else {
		return false;
	}
}

bool process(PDU &pkt) {
    if (pkt.pdu_type() == pkt.RADIOTAP) {
        //Start processing of RadioTap Packets
        PDU *innerPkt = pkt.inner_pdu();
        if (is_management_frame(innerPkt->pdu_type())) {
            if (in_interface == "hwsim0" && innerPkt->pdu_type() != pkt.DOT11_BEACON) {
                return true;
            }

            Dot11ManagementFrame &management_frame = innerPkt->rfind_pdu<Dot11ManagementFrame>();
            Dot11::address_type source = management_frame.addr2();
            if (source == HOSTAPD || source == BROADCAST) {
                //Add an ethernet frame and send over iface
                EthernetII to_send = EthernetII(WARP, PC_ENGINE) / management_frame;
                sender->send(to_send);
            }
        } else if (pkt.pdu_type() == pkt.DOT11_DATA) {
            Dot11Data &dataPkt = innerPkt->rfind_pdu<Dot11Data>();
            Dot11::address_type source = dataPkt.addr2();
            if(source == HOSTAPD || source == BROADCAST) {
                //Add an ethernet frame and send over iface to warp
                EthernetII packetOut = EthernetII(WARP, PC_ENGINE) / dataPkt;
                sender->send(packetOut);
            }
        } else {//We should not be handling 802.11 Control packets for now
            cerr << "Inner Layer is " << pkt.inner_pdu()->pdu_type() << "! Skipping..." << endl;
        }
    } else {
        cerr << "Error: Non RadioTap Packet Detected!" << endl;
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
	if (argc == 3) {
		set_in_interface(argv[1]);
        set_out_interface(argv[2]);
	} else {
        set_in_interface("hwsim0");
        set_out_interface("eth0");
	}

	sniff(in_interface);
}