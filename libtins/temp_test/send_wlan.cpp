#include <stdio.h>
#include <string>

#include <tins/tins.h>

using namespace Tins;
using namespace std;

PacketSender *sender;

bool process(PDU &pkt) {
    if (pkt.pdu_type() == pkt.ETHERNET_II || pkt.pdu_type() == pkt.IEEE802_3) {
        EthernetII &eth = pkt.rfind_pdu<EthernetII>();
        //For now forward everything
        if (1 == 1) {
            if (eth.payload_type() == 0x0800 || eth.payload_type() == 0x0806) {
                //Add an ethernet frame and send over iface to warp
                sender->send(pkt);
                cout << "Sent 1 frame.\n";
            }
        }
    } else {
        cerr << "Error: Non EthernetII Packet Detected!" << endl;
    }
    return true;
}

int main(int argc, char *argv[]) {
    sender = new PacketSender("eth1");
	Sniffer sniffer("wlan0", Sniffer::PROMISC);
    sniffer.sniff_loop(process);
}