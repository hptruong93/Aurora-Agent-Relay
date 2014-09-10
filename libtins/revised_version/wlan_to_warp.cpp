#include <stdio.h>
#include <string>

#include "config.h"
#include "util.h"
#include <tins/tins.h>
#include "../send_receive_module/fragment_sender.h"
#include "../warp_protocol/warp_protocol.h"

using namespace Tins;
using namespace Config;
using namespace std;

PacketSender *sender;
FragmentSender* fragment_sender;
string in_interface;

bool process(PDU &pkt) {
    if (pkt.pdu_type() == pkt.ETHERNET_II || pkt.pdu_type() == pkt.IEEE802_3) {
        //For now forward everything
        if (1 == 1) {
            //Add an ethernet frame and send over iface to warp
            //-----------------> Create WARP transmit info
            WARP_protocol::WARP_transmit_struct* transmit_info = WARP_protocol::get_default_transmit_struct();
            transmit_info->retry = MAX_RETRY;
            transmit_info->payload_size = (uint16_t) pkt.size();

            //-----------------> Create WARP layer and append at the end
            cout << "Original ethernet packet size is " << pkt.size() << endl;
            fragment_sender->send(pkt, TYPE_TRANSMIT, SUBTYPE_DATA_TRANSMIT, transmit_info);

            //-----------------> Clean up
            free(transmit_info);
            cout << "Sent 1 data packet" << endl;
        }
    } else {
        cerr << "Error: Non EthernetII Packet Detected!" << endl;
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
	if (argc >= 3) {
		set_in_interface(argv[1]);
        set_out_interface(argv[2]);
        cout << "Init pc to warp from " << argv[1] << " to " << argv[2] << endl;

        if (argc == 4) {
            Config::HOSTAPD = Tins::HWAddress<6>(argv[3]);
            cout << "hostapd mac is " << argv[3] << endl;
        }        
	} else {
        set_in_interface("hwsim0");
        set_out_interface("eth0");
        cout << "Init pc to warp from hwsim0 to eth0" << endl;
	}

    fragment_sender = new FragmentSender(sender);
	sniff(in_interface);
}