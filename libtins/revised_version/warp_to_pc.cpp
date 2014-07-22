
#include <stdio.h>
#include <string.h>

#include <tins/tins.h>
#include "config.h"
#include "../warp_protocol/warp_protocol.h"

using namespace Tins;
using namespace std;
using namespace Config;

PacketSender *sender;
string in_interface;

bool process(PDU &pkt) {
    //cout << "Start" << endl;
    EthernetII &ethernet_packet = pkt.rfind_pdu<EthernetII>();
    //cout << "Found" << endl;
    if (ethernet_packet.src_addr() == Config::WARP && ethernet_packet.dst_addr() == Config::PC_ENGINE) {
        //cout << "Inside" << endl;
        WARP_protocol &warp_layer = ethernet_packet.rfind_pdu<WARP_protocol>();
        //cout << "Found protocol" << endl;
        uint8_t* warp_layer_buffer = warp_layer.get_buffer();
        uint32_t processed_bytes = WARP_protocol::process_warp_layer(warp_layer_buffer + 0); 

        RawPDU payload(warp_layer_buffer + processed_bytes, warp_layer.header_size() - processed_bytes);
        //cout << "Here" << endl;
        // for (uint8_t i = 0; i < warp_layer.header_size() - processed_bytes; i++) {
        //     printf("%d\n", warp_layer_buffer[processed_bytes + i]);
        // }

        //Put in radio tap and send to output
        RadioTap header(default_radio_tap_buffer, sizeof(default_radio_tap_buffer));
        RadioTap to_send = header /  payload;

        sender->send(to_send);
        cout << "Sent 1 packet" << endl;
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

    if (argc == 3) {
        set_in_interface(argv[1]);
        set_out_interface(argv[2]);
    } else {
        set_in_interface("eth0");
        set_out_interface("mon.wlan1");
    }

    sniff(in_interface);
}