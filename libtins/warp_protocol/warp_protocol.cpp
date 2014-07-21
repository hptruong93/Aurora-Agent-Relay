#include <cstring>
#include <cassert>
#include "warp_protocol.h"
#include "tins/constants.h"
#include "tins/exceptions.h"
#include "tins/rawpdu.h"

namespace Tins {
    
    WARP_protocol::WARP_protocol(const uint8_t *buffer, uint32_t total_sz, uint32_t inner_size) {
        if(total_sz > sizeof(data))
            throw malformed_packet();

        memset(data, 0, sizeof(data));
        memcpy(data, buffer, total_sz);
        total_length = total_sz;

        if (inner_size > 0) {
            inner_pdu(new RawPDU(buffer + total_sz, inner_size));
        }
    }

    WARP_protocol::WARP_protocol(const uint8_t *buffer, uint32_t total_sz) {
        if(total_sz > sizeof(data))
            throw malformed_packet();
        memcpy(data, buffer, total_sz);
        total_length = total_sz;
    }
    
    //Returns Header Size (Getter)
    uint32_t WARP_protocol::header_size() const {
        return total_length;
    }
    
    //Serialize entire packet (only assert if running in debug mode)
    void WARP_protocol::write_serialization(uint8_t *buffer, uint32_t total_sz, const PDU *parent) {
#ifdef TINS_DEBUG
        assert(total_sz >= sizeof(header_size()));
#endif
        memcpy(buffer, &(data[0]), total_sz);
    }
    
}