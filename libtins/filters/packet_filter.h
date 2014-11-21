#ifndef PACKET_FILTER_H
#define PACKET_FILTER_H

#include <tins/tins.h>

namespace PacketFilter {
	//Template filter
	class PacketFilter {
        public:
            PacketFilter();
            PacketFilter(PacketFilter* extra);
            bool filter(Tins::PDU *packet);
            ~PacketFilter();
            
        protected:
            virtual bool internal_filtering(Tins::PDU *packet);

        private:
        	PacketFilter* decorator;
    };
}

#endif