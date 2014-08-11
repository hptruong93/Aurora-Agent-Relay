#include "tins/tins.h"
#include <cstdlib>
//#include "warp_transmit.h"
#include "warp_protocol.h"

using namespace std;
using namespace Tins;

HWAddress<6> ETHSRC("a0:a1:a2:a3:a4:a5"); //PCEngine --> hostapd
HWAddress<6> ETHDST("d0:d1:d2:d3:d4:d5"); //WARP

int main() {
    //Allocators::register_allocator<EthernetII, Tins::WARP_protocol>(0x8ae);
    // uint8_t size = 42;
    // uint8_t* buffer = (uint8_t*) std::malloc(size);
    // std::memset(buffer, 9, size);

    // WARP_protocol transmit(buffer, size);

    // const uint8_t buff_temp[] = {0x08, 0x08, 0x08};

    // EthernetII ether = EthernetII(ETHDST, ETHSRC) / transmit / RawPDU(buff_temp, 3);
    // ether.payload_type(0x8ae);
    // PacketSender sender("eth0");

    // sender.send(ether);

    auto it = ETHSRC.begin();

    for (; it != ETHSRC.end(); it++) {
        uint8_t a = *it;
        printf("%02x-", a);
    }
    return 0;
}