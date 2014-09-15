#include <cstring>
#include <cassert>
#include <cstdlib>
#include "warp_protocol.h"
#include "tins/constants.h"
#include "tins/exceptions.h"
#include "tins/rawpdu.h"
#include "tins/tins.h"
#include "../revised_version/util.h"

using namespace std;

#define RESERVED_FRAGMENT_ID                     255

namespace Tins {
    
    uint32_t WARP_protocol::process_warp_layer(uint8_t* input_buffer, WARP_protocol::WARP_transmit_struct* transmit_result) {
        if (input_buffer[TYPE_INDEX] == TYPE_TRANSMIT) {
            //For future usage???
            uint8_t i;
            for (i = 0; i < 6; i++) {
                transmit_result->bssid[i] = (input_buffer + BSSID_INDEX)[i];
            }
            transmit_result->flag = input_buffer[FLAG_INDEX];
            transmit_result->retry = input_buffer[RETRY_INDEX];
            transmit_result->payload_size = (input_buffer[DATA_LENGTH_MSB_INDEX] << 8) + (input_buffer[DATA_LENGTH_LSB_INDEX]);
            return FRAGMENT_INFO_INDEX;
        } else {
            return 0;
        }
    }

    WARP_protocol::WARP_transmit_struct* WARP_protocol::get_default_transmit_struct(Tins::HWAddress<6> bssid) {
        WARP_transmit_struct* output = (WARP_transmit_struct*) calloc(sizeof(WARP_transmit_struct), 0);
        
        output->flag = DEFAULT_TRANSMIT_FLAG;
        output->retry = MAX_RETRY;
        output->payload_size = 0;
        convert_mac(&(output->bssid[0]), bssid);

        return output;
    }

    WARP_protocol::WARP_mac_control_struct* WARP_protocol::get_default_mac_control_struct(Tins::HWAddress<6> mac_address) {
        WARP_mac_control_struct* output = (WARP_mac_control_struct*) calloc(sizeof(WARP_mac_control_struct), 0);

        output->operation_code = DEFAULT_MAC_CONTROL_OPERATION_CODE;
        convert_mac(&(output->mac_address[0]), mac_address);

        return output;
    }

    WARP_protocol::WARP_transmission_control_struct* WARP_protocol::get_default_transmission_control_struct(Tins::HWAddress<6> bssid) {
        WARP_transmission_control_struct* output = (WARP_transmission_control_struct*) calloc(sizeof(WARP_transmission_control_struct), 0);

        output->disabled = DEFAULT_TRANSMISSION_CONTROL_DISABLED;
        output->tx_power = DEFAULT_TRANSMISSION_CONTROL_TX_POWER;
        output->channel = DEFAULT_TRANSMISSION_CONTROL_CHANNEL;
        output->rate = DEFAULT_TRANSMISSION_CONTROL_RATE;
        output->hw_mode = DEFAULT_TRANSMISSION_CONTROL_HW_MODE;
        convert_mac(&(output->bssid[0]), bssid);

        return output;
    }

    WARP_protocol::WARP_fragment_struct* WARP_protocol::generate_fragment_struct() {
        static uint8_t id_counter = 0;

        WARP_fragment_struct* output = (WARP_fragment_struct*) calloc(1, sizeof(WARP_fragment_struct));

        output->id = id_counter;
        id_counter++;
        if (id_counter == RESERVED_FRAGMENT_ID) {
            id_counter = 0;
        }

        output->fragment_number = 1;
        output->total_number_fragment = 1;
        output->byte_offset = 0;

        return output;
    }

    WARP_protocol* WARP_protocol::create_transmit(WARP_transmit_struct* info, WARP_fragment_struct* fragment_info, uint8_t subtype) {
        uint8_t* buffer;
        uint8_t buffer_length;

        buffer_length = 2 + 10 + 5;
        buffer = (uint8_t*) std::malloc(buffer_length);

        //Header
        buffer[TYPE_INDEX] = 0x01;
        buffer[SUBTYPE_INDEX] = subtype;

        //Transmit
        memcpy(&(buffer[2]), &(info->bssid[0]), 6);
        buffer[FLAG_INDEX] = info->flag;
        buffer[RETRY_INDEX] = info->retry;
        

        info->payload_size += FRAGMENT_INFO_LENGTH;
        if (fragment_info == NULL) {
            cout << "NULL fragment info detected..." << endl;
        }

        buffer[12] = fragment_info->id;
        buffer[13] = fragment_info->fragment_number;
        buffer[14] = fragment_info->total_number_fragment;
        buffer[15] = ((fragment_info->byte_offset) >> 8) & 0xff;
        buffer[16] = (fragment_info->byte_offset) & 0xff;

        buffer[DATA_LENGTH_MSB_INDEX] = (info->payload_size >> 8) & 0xff;
        buffer[DATA_LENGTH_LSB_INDEX] = info->payload_size & 0xff;

        WARP_protocol* output = new WARP_protocol(buffer, buffer_length);
        std::free(buffer);
        return output;
    }

    WARP_protocol* WARP_protocol::create_mac_control(WARP_mac_control_struct* info) {
        uint8_t* buffer;
        uint8_t buffer_length = WARP_PROTOCOL_HEADER_LENGTH + MAC_CONTROL_ELEMENT_LENGTH;
        
        buffer = (uint8_t*) std::malloc(buffer_length);

        //Header
        buffer[TYPE_INDEX] = TYPE_CONTROL;
        buffer[SUBTYPE_INDEX] = SUBTYPE_MAC_ADDRESS_CONTROL;

        //MAC control
        buffer[2] = info->operation_code;
        memcpy(buffer + WARP_PROTOCOL_HEADER_LENGTH + 1, &(info->mac_address[0]), 6);

        return new WARP_protocol(buffer, buffer_length);
    }

    WARP_protocol* WARP_protocol::create_transmission_control(WARP_transmission_control_struct* info) {
        uint8_t* buffer;
        uint8_t buffer_length;

        buffer_length = WARP_PROTOCOL_HEADER_LENGTH + TRANSMISSION_CONTROL_ELEMENT_LENGTH;
        buffer = (uint8_t*) std::malloc(buffer_length);

        //Header
        buffer[TYPE_INDEX] = TYPE_CONTROL;
        buffer[SUBTYPE_INDEX] = SUBTYPE_TRANSMISSION_CONTROL;

        //Control
        memcpy(buffer + WARP_PROTOCOL_HEADER_LENGTH, &(info->bssid[0]), 6);
        buffer[DISABLED_INDEX] = info->disabled;
        buffer[TX_POWER_INDEX] = info->tx_power;
        buffer[CHANNEL_INDEX] = info->channel;
        buffer[RATE_INDEX] = info->rate;
        buffer[HW_MODE_INDEX] = info->hw_mode;

        return new WARP_protocol(buffer, buffer_length);
    }

    /*********************************************************************************************************
                                        End of static methods
    *********************************************************************************************************/

    WARP_protocol::WARP_protocol(const uint8_t *data, uint32_t total_sz) {
        // buffer = (uint8_t*) std::malloc(total_sz * 0 + 2048);
        //buffer = new uint8_t[total_sz];
        size = total_sz;
        for (uint32_t i = 0; i < total_sz; i++) {
            buffer[i] = data[i];
        }
    }
    
    WARP_protocol::~WARP_protocol() {
    }

    void WARP_protocol::free_buffer() {
        // free(buffer);
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