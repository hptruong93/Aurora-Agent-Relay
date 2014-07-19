
#include <stdio.h>
#include <string.h>

#include <tins/tins.h>
#include "config.h"

using namespace Tins;
using namespace std;
using namespace Config;

PacketSender *sender;
string in_interface;

bool process(PDU &pkt) {
    Dot3 &ethernet_packet = pkt.rfind_pdu<Dot3>();
    if (ethernet_packet.src_addr() == WARP && ethernet_packet.dst_addr() == PC_ENGINE) {
        PDU *payload = ethernet_packet.inner_pdu();

        //Put in radio tap and send to output
        RadioTap header(default_radio_tap_buffer, sizeof(default_radio_tap_buffer));
        RadioTap to_send = header /  (*payload);

        sender->send(to_send);
        cout << "Sent 1 packet\n";
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
        set_in_interface("eth0");
        set_out_interface("mon.wlan1");
    }

    sniff(in_interface);
}