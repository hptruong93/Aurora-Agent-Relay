/* packet_crafter.cpp: Choose which packets to craft and send
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

//Global Variables
NetworkInterface iface("eth3");
HWAddress<6> WIFISRC("c0:c1:c2:c3:c4:c5"); //Random for now
HWAddress<6> WIFIDST("02:00:00:00:00:00"); //Virtual Wifi Interface
HWAddress<6> ETHDST("0a:00:27:00:00:00"); //PCEngine
HWAddress<6> ETHSRC("a0:a1:a2:a3:a4:a5"); //WARP
PacketSender sender;

//Main Code
int main(int argc, char *argv[]) {
    Allocators::register_allocator<EthernetII, WARPControlPDU>(0x808);
    bool exitLoop = false;
    int choice;
    while(!exitLoop) {
        cout << "\n\nPacket Crafter\n Select one of the following options to continue:" << endl;
        cout << "\t1. Send Association Request Packet" << endl;
        cout << "\t2. Send Reassociation Request Packet" << endl;
        cout << "\t3. Send Probe Request Packet" << endl;
        cout << "\t4. Send Disassociation Packet" << endl;
        cout << "\t5. Send Authentication Packet" << endl;
        cout << "\t6. Exit" << endl;
        cin >> choice;
        switch(choice) {
            case 1: {
                cout << "Sending Association Request Packet..." << endl;
                EthernetII assocPkt = EthernetII(ETHDST, ETHSRC) / WARPControlPDU() / 	Dot11AssocRequest(WIFIDST, WIFISRC);
                sender.send(assocPkt, iface);
                break;
            }
            case 2: {
                cout << "Sending Reassociation Request Packet..." << endl;
                EthernetII rAssocPkt = EthernetII(ETHDST, ETHSRC) / WARPControlPDU() / Dot11ReAssocRequest(WIFIDST, WIFISRC);
                sender.send(rAssocPkt, iface);
                break;
            }
            case 3: {
                cout << "Sending Probe Request Packet..." << endl;
                EthernetII probePkt = EthernetII(ETHDST, ETHSRC) / WARPControlPDU() / Dot11ProbeRequest(WIFIDST, WIFISRC);
                sender.send(probePkt, iface);
                break;
            }
            case 4: {
                cout << "Sending Disassociation Packet..." << endl;
                EthernetII dAssocPkt = EthernetII(ETHDST, ETHSRC) / WARPControlPDU() / Dot11Disassoc(WIFIDST, WIFISRC);
                sender.send(dAssocPkt, iface);
                break;
            }
            case 5: {
                cout << "Sending Authentication Packet..." << endl;
                EthernetII authPkt = EthernetII(ETHDST, ETHSRC) / WARPControlPDU() / Dot11Authentication(WIFIDST, WIFISRC);
                sender.send(authPkt, iface);
                break;
            }
            case 6: {
                cout << "Exiting Program..." << endl;
                exitLoop = true;
                break;
            }
            default: {
                cout << "Error: Unrecognized command..." << endl;
            }
        }
    }
}
