/* wifiToEth.cpp: Sniffs on ethernet and forwards relevent packets to Virtual Wifi Interface
 * Kevin Han
 * McGill University Broadband Communications Research Lab
 * Smart Applications on Virtual Infrastructure Team
 * May 2014 */
 
#include <iostream>
#include <string>
#include <vector>
#include <stddef.h>
#include <tins/tins.h>
#include "WARPControl_pdu.h"

//Namespaces
using namespace std;
using namespace Tins;

//Forward Declarations
bool callback(const PDU &pkt);
string PDUTypeToString(int PDUTypeFlag);

//Global Variables
NetworkInterface iface("eth1");
NetworkInterface monIface("hwsim0");
NetworkInterface wlanIface("wlan0");
HWAddress<6> WLAN0("02:00:00:00:00:00");
HWAddress<6> BROADCAST("ff:ff:ff:ff:ff:ff");
HWAddress<6> ETHDST("a4:ba:db:fd:52:7a"); //PCEngine
string FILTER("eth dst a4:ba:db:fd:52:7a");
PacketSender sender;

//Main Code
int main(int argc, char *argv[]) {
    Allocators::register_allocator<EthernetII, WARPControlPDU>(0x808);
    Sniffer sniffer("eth1", 65535, true, FILTER);
    sniffer.sniff_loop(callback);
}

bool callback(const PDU &pkt) {
    if (pkt.pdu_type() == pkt.ETHERNET_II) {
        //Start processing Ethernet Frames (Strip Ethernet, Append RadioTap, Forward)
        PDU *innerPkt = pkt.inner_pdu();
        RadioTap packetOut = RadioTap() / (*innerPkt);
        sender.send(packetOut, monIface);
    }
    else
        cerr << "Error: Non Ethernet Type Packet Detected!" << endl;
    return true;
}

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
