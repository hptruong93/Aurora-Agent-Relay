#ifndef DOT11_FILTER_H
#define DOT11_FILTER_H

#include <tins/tins.h>
#include "packet_filter.h"

namespace PacketFilter {
	class Dot11Filter : public PacketFilter {
        public:
            Dot11Filter(Tins::HWAddress<6> filter_mac, Tins::small_uint<2> type, Tins::small_uint<2> subtype);
            
        protected:
            bool internal_filtering(Tins::PDU *packet);

        private:
        	Tins::HWAddress<6> mac;
        	Tins::small_uint<2> type;
        	Tins::small_uint<2> subtype;
    };
}

#endif