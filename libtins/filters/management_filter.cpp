#include "management_filter.h"

using namespace Tins;

namespace PacketFilter {
	ManagementFilter::ManagementFilter() : PacketFilter(nullptr) {};

	bool ManagementFilter::internal_filtering(PDU *packet) {
		//Return true if packet is management packet
		try {
			Dot11 &frame = packet->rfind_pdu<Dot11>();
			if (frame.type() == Dot11::MANAGEMENT) {
				return true;
			} else {
				return false;
			}
		} catch (...) {//Error for creating packet?
			return false;
		}
	}
}