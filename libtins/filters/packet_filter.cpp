#include "packet_filter.h"

using namespace Tins;

namespace PacketFilter {

	PacketFilter::PacketFilter() : decorator(nullptr) {};
	PacketFilter::PacketFilter(PacketFilter* extra) : decorator(extra) {};

	PacketFilter::~PacketFilter() {
		if (this->decorator != nullptr) {
			this->decorator->~PacketFilter(); //Double free problem?
		}
	}

	bool PacketFilter::filter(PDU* packet) {
		bool internal_result = true;
		if (this->decorator != nullptr) {
			internal_result = internal_result && this->decorator->internal_filtering(packet);
		}

		internal_result = internal_result && internal_filtering(packet);
		
		return internal_result;
	}

	bool PacketFilter::internal_filtering(PDU* packet) {
		//Do nothing
		return true;
	}
}