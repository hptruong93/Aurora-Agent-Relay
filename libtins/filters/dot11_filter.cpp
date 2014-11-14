#include "dot11_filter.h"

using namespace Tins;

namespace PacketFilter {
	Dot11Filter::Dot11Filter(Tins::HWAddress<6> filter_mac, small_uint<2> type, small_uint<2> subtype) : 
		PacketFilter(nullptr) , mac(filter_mac), type(type), subtype(subtype) {}; //Do nothing

	bool Dot11Filter::internal_filtering(PDU *packet) {
		//Return true if packet is probe request from the MAC address
		//or probe response, to the MAC address
		try {
			Dot11 &dot11 = packet->rfind_pdu<Dot11>();
			//Check if probe request or probe response
			if (dot11.type() == this->type) {
				if (dot11.subtype() == this->subtype) {
					return true;
				}
			}

			return false;
		} catch (...) {//Error for creating packet?
			return false;
		}
	}
}