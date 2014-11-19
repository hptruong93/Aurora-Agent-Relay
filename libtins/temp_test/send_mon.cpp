#include <stdio.h>
#include <string>

#include <tins/tins.h>
#include "../warp_protocol/warp_protocol.h"

using namespace Tins;
using namespace std;

Tins::HWAddress<6> BROADCAST("ff:ff:ff:ff:ff:ff");
Tins::HWAddress<6> HOSTAPD1("40:d8:55:04:22:84");
Tins::HWAddress<6> HOSTAPD2("40:d8:55:04:22:86");
Tins::HWAddress<6> PC_ENGINE("00:0D:B9:34:17:29");
Tins::HWAddress<6> WARP("40:d8:55:04:22:84");
Tins::HWAddress<6> DEFAULT_MAC("00:00:00:00:00:00");


PacketSender* sender;

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
            Dot11ManagementFrame &management_frame = innerPkt->rfind_pdu<Dot11ManagementFrame>();
            Dot11::address_type source = management_frame.addr2();
            Dot11::address_type dest = management_frame.addr1();

            if (source == HOSTAPD1 || source == HOSTAPD2 || source == BROADCAST) {
                //Add an ethernet frame and send over iface
                EthernetII to_send(WARP, PC_ENGINE);

                uint8_t warp_header[] = {0x01, 0x00, 0x00, 0x00};
                warp_header[2] = (management_frame.size() >> 8) & 0xff;
                warp_header[3] = (management_frame.size()) & 0xff;

                WARP_protocol header(warp_header, 4);

                to_send = to_send / header / management_frame;
                to_send.payload_type(0x8ae);
                sender->send(to_send);
                cout << "Sent 1 management packet" << endl;
            }
        } else {//802.11 Control packets
            cout << "Whaaat??\n";
        }
    } else {
        cerr << "Error: Non RadioTap Packet Detected!" << endl;
    }
    return true;
}

int main(int argc, char *argv[]) {
    sender = new PacketSender("eth1");
    Sniffer sniffer("hwsim0", Sniffer::PROMISC);
    sniffer.sniff_loop(process);
}
