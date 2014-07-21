#include <cstring>
#include <cassert>
#include "warp_protocol.h"
#include "tins/constants.h"
#include "tins/exceptions.h"
#include "tins/rawpdu.h"

namespace Tins {
    
    WARP_protocol::WARP_protocol(const uint8_t* data, uint32_t sz) : buffer(data, data + sz) { }
    
    //Serialize entire packet (only assert if running in debug mode)
    void WARP_protocol::write_serialization(uint8_t *data, uint32_t total_sz, const PDU *parent) {
#ifdef TINS_DEBUG
        assert(total_sz >= sizeof(header_size()));
#endif
        std::copy(buffer.begin(), buffer.end(), data);
    }
    
}