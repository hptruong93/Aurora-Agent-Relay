#include "tins/tins.h"
#include <cstdlib>
//#include "warp_transmit.h"
#include "warp_protocol.h"

using namespace std;
using namespace Tins;

HWAddress<6> ETHSRC("a0:a1:a2:a3:a4:a5"); //PCEngine --> hostapd
HWAddress<6> ETHDST("d0:d1:d2:d3:d4:d5"); //WARP
HWAddress<6> TEST_MAC("00:00:00:00:00:00"); // TEST MAC ADDRESS

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

    WARP_protocol::WARP_mac_control_struct* test_ptr = WARP_protocol::get_default_mac_control_struct(TEST_MAC);
    WARP_protocol* test = WARP_protocol::create_mac_control(test_ptr);
    printf("Yes?? %d\n", test->get_buffer()[0]);

    uint8_t data[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    RawPDU aa(data, 10);
    PDU::serialization_type serial = aa.serialize();
    uint8_t* buffer = &serial[0];
    uint8_t i = 0;
    for (; i < 10; i++) {
        printf("%d-", buffer[i]);
    }

    return 0;
}