#include <stdio.h>
#include <string>

#include "config.h"
#include "util.h"
#include <tins/tins.h>
#include "../warp_protocol/warp_protocol.h"

using namespace Tins;
using namespace Config;
using namespace std;

#define WARP_LAYER_ENABLE

PacketSender *sender;
string in_interface;

bool process(PDU &pkt) {
    if (pkt.pdu_type() == pkt.ETHERNET_II) {
        EthernetII &packet = pkt.rfind_pdu<EthernetII>();

        //For now forward everything
        if (1 == 1) {
            //Add an ethernet frame and send over iface to warp
            EthernetII to_send = EthernetII(WARP, PC_ENGINE);
            to_send.payload_type(WARP_PROTOCOL_TYPE);

#ifdef WARP_LAYER_ENABLE
            //-----------------> Create WARP transmit info
            WARP_protocol::WARP_transmit_struct transmit_info;
            transmit_info.power = DEFAULT_TRANSMIT_POWER;
            transmit_info.rate = DEFAULT_TRANSMIT_RATE;
            transmit_info.channel = DEFAULT_TRANSMIT_CHANNEL;
            transmit_info.flag = DEFAULT_TRANSMIT_FLAG;
            transmit_info.retry = MAX_RETRY;
            transmit_info.payload_size = (uint16_t) packet.size();

            //-----------------> Create WARP layer and append at the end
            WARP_protocol* warp_layer = WARP_protocol::create_transmit(&transmit_info, 1);
            to_send = to_send / (*warp_layer) / (pkt);
#else
            to_send = to_send / pkt;
#endif

            sender->send(to_send);

            //-----------------> Clean up
#ifdef WARP_LAYER_ENABLE
            delete warp_layer;
#endif
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
	if (argc == 3) {
		set_in_interface(argv[1]);
        set_out_interface(argv[2]);
        cout << "Init pc to warp from " << argv[1] << " to " << argv[2] << endl;
	} else {
        set_in_interface("hwsim0");
        set_out_interface("eth0");
        cout << "Init pc to warp from hwsim0 to eth0" << endl;
	}

	sniff(in_interface);
}