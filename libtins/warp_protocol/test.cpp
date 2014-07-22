#include "tins/tins.h"
//#include "warp_transmit.h"
#include "warp_protocol.h"

using namespace std;
using namespace Tins;

HWAddress<6> ETHSRC("a0:a1:a2:a3:a4:a5"); //PCEngine --> hostapd
HWAddress<6> ETHDST("d0:d1:d2:d3:d4:d5"); //WARP

int main() {
	//Allocators::register_allocator<EthernetII, Tins::WARP_protocol>(0x8ae);
	const uint8_t buffer[] = {0, 1, 1, 2, 7};
	WARP_protocol transmit(buffer, sizeof(buffer));

	EthernetII ether = EthernetII(ETHDST, ETHSRC) / transmit / IP("1.1.1.1", "2.2.2.2");
	PacketSender sender("eth0");

	for (; ; ) {
		sender.send(ether);
	}
	return 0;
}