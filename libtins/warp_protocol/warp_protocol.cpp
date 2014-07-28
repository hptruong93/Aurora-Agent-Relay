#include <cstring>
#include <cassert>
#include <cstdlib>
#include "warp_protocol.h"
#include "tins/constants.h"
#include "tins/exceptions.h"
#include "tins/rawpdu.h"
#include "tins/tins.h"

using namespace std;

namespace Tins {
    
    uint32_t WARP_protocol::process_warp_layer(uint8_t* input_buffer) {
        if (input_buffer[TYPE_INDEX] == TYPE_TRANSMIT) {
            return 9;
        } else {
            return 0;
        }
    }

    WARP_protocol* WARP_protocol::create_transmit(WARP_transmit_struct* info) {
        uint8_t buffer[9];

        //Header
        buffer[TYPE_INDEX] = 0x01;
        buffer[SUBTYPE_INDEX] = 0x00;

        //Transmit
        buffer[2] = info->power;
        buffer[3] = info->rate;
        buffer[4] = info->channel;
        buffer[5] = info->flag;
        buffer[6] = info->retry;
        buffer[7] = (info->payload_size >> 8) & 0xff;
        buffer[8] = info->payload_size & 0xff;

        return new WARP_protocol(buffer, 9);
    }

    WARP_protocol* WARP_protocol::create_mac_control(uint8_t operation_code, uint8_t* mac_address) {
        uint8_t buffer[9];

        //Header
        buffer[TYPE_INDEX] = 0x02;
        buffer[SUBTYPE_INDEX] = 0x02;

        //MAC control
        buffer[2] = operation_code;
        memcpy(buffer + WARP_PROTOCOL_HEADER_LENGTH + 1, mac_address, 6);

        return new WARP_protocol(buffer, 9);
    }

    /*********************************************************************************************************
                                        End of static methods
    *********************************************************************************************************/

    WARP_protocol::WARP_protocol(const uint8_t *data, uint32_t total_sz) {
        buffer = (uint8_t*) std::malloc(total_sz);
        //buffer = new uint8_t[total_sz];
        size = total_sz;
        for (uint32_t i = 0; i < total_sz; i++) {
            buffer[i] = data[i];
        }
    }
    
    WARP_protocol::~WARP_protocol() {
        //cout << "Deleting " << size << endl;
        //delete [] buffer;
    }

    //Serialize entire packet (only assert if running in debug mode)
    void WARP_protocol::write_serialization(uint8_t *data, uint32_t total_sz, const PDU *parent) {
#ifdef TINS_DEBUG
        assert(total_sz >= header_size());
#endif
        //std::copy(buffer.begin(), buffer.end(), data);
        if (total_sz > header_size()) {
            memmove(data, buffer, header_size());
        } else {
            memmove(data, buffer, total_sz);
        }
    }
}