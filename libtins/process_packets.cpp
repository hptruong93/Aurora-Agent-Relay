/* process_packets.cpp: Process incoming packets
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

//Global Variables
NetworkInterface iface("wlan0");
HWAddress<6> WLAN0("02:00:00:00:00:00");
HWAddress<6> BROADCAST("ff:ff:ff:ff:ff:ff");
HWAddress<6> ETHSRC("0a:00:27:00:00:00"); //PCEngine
HWAddress<6> ETHDST("a0:a1:a2:a3:a4:a5"); //WARP
PacketSender sender;

//Main Code
int main(int argc, char *argv[]) {
    Allocators::register_allocator<EthernetII, WARPControlPDU>(0x808);
    cout << "Starting Packet Processing..." << endl;
    Sniffer sniffer("eth3", 65535, true, "");
    sniffer.sniff_loop(callback);
}

bool callback(const PDU &pkt) {
    if (pkt.pdu_type() == pkt.ETHERNET_II) {
        //Check source/destination and Strip WARP Control Header
        PDU *innerPkt = pkt.inner_pdu();
        const EthernetII &ethPkt = pkt.rfind_pdu<EthernetII>();
        if ((ethPkt.src_addr() == ETHSRC) && (ethPkt.dst_addr() == ETHDST) && (innerPkt->pdu_type() == pkt.USER_DEFINED_PDU)) {
            PDU *Dot11Pkt = innerPkt->inner_pdu();
            sender.send(*Dot11Pkt, iface);
        }
        else
            cout << "Error: Malformed WARP Control Header or Bad Src/Dst address...\nSkipping..." << endl;
    }
    else
        cout << "Non Ethernet Packet Detected...\nSkipping..." << endl;
    return true;
}
