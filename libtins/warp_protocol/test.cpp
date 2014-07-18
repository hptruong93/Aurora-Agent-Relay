#include "tins/tins.h"
#include "warp_transmit.h"

using namespace std;
using namespace Tins;

// HWAddress<6> ETHSRC("a0:a1:a2:a3:a4:a5"); //PCEngine --> hostapd
// HWAddress<6> ETHDST("d0:d1:d2:d3:d4:d5"); //WARP

int main() {
	Allocators::register_allocator<EthernetII, Tins::WARP_transmit>(0x8ae);
	// WARP_transmit transmit;
	// EthernetII ether = EthernetII(ETHDST, ETHSRC) / transmit;
	// PacketSender sender("eth0");

	// for (uint8_t i = 0; i < 10; i++) {
	// 	sender.send(ether);
	// }
	return 0;
}