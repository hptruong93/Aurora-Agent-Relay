#ifndef MANAGEMENT_FILTER_H
#define MANAGEMENT_FILTER_H

#include <tins/tins.h>
#include "packet_filter.h"

namespace PacketFilter {
	class ManagementFilter : public PacketFilter {
        public:
            ManagementFilter();
            void set_decorator(PacketFilter* decorator) {this->decorator = decorator;};
            
        protected:
            bool internal_filtering(Tins::PDU *packet);

        private:
        	PacketFilter* decorator;
    };
}

#endif