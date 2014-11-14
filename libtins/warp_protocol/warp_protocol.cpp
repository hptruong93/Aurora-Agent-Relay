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
    uint8_t WARP_protocol::check_warp_layer_type(uint8_t* input_buffer)
    {
        return input_buffer[TYPE_INDEX] <= TYPE_TRANSMIT ? input_buffer[TYPE_INDEX] : input_buffer[SUBTYPE_INDEX];
    }
    
    uint32_t WARP_protocol::process_warp_layer(uint8_t* input_buffer,  void* result) {
        if (input_buffer[TYPE_INDEX] == TYPE_TRANSMIT) {
            ((WARP_protocol::WARP_transmit_struct*)result)->payload_size = (input_buffer[DATA_LENGTH_MSB_INDEX] << 8) + (input_buffer[DATA_LENGTH_LSB_INDEX]);
            return HEADER_OFFSET + 2;
        } else if (input_buffer[TYPE_INDEX] == TYPE_CONTROL) {
            switch(input_buffer[SUBTYPE_INDEX])
            {
                case SUBTYPE_MAC_ADDRESS_CONTROL:
                    ((WARP_protocol::WARP_mac_control_struct*)result)->operation_code = input_buffer[OPERTAION_CODE_INDEX];
                    return HEADER_OFFSET + 1;
                case SUBTYPE_TRANSMISSION_CONTROL:
                    ((WARP_protocol::WARP_transmission_control_struct*)result)->operation_code = input_buffer[TRANSMISSION_OPERATION_CODE_INDEX];
                    return HEADER_OFFSET + 1;
            }
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

        output->total_num_element = 1;
        output->disabled = DEFAULT_TRANSMISSION_CONTROL_DISABLED;
        output->tx_power = DEFAULT_TRANSMISSION_CONTROL_TX_POWER;
        output->channel = DEFAULT_TRANSMISSION_CONTROL_CHANNEL;
        output->rate = DEFAULT_TRANSMISSION_CONTROL_RATE;
        output->hw_mode = DEFAULT_TRANSMISSION_CONTROL_HW_MODE;
        output->operation_code = DEFAULT_TRNASMISSION_CONTROL_OPERATION_CODE;

        convert_mac(&(output->bssid[0]), bssid);

        return output;
    }

    WARP_protocol::WARP_fragment_struct* WARP_protocol::generate_fragment_struct() {
        // static uint8_t id_counter = 0;

        WARP_fragment_struct* output = (WARP_fragment_struct*) calloc(1, sizeof(WARP_fragment_struct));

        std::lock_guard<std::mutex> id_lock(Tins::Fragment_Id_Mux);

        output->id = Tins::Fragment_Id;
        Tins::Fragment_Id++;
        if (Tins::Fragment_Id == RESERVED_FRAGMENT_ID) {
            Tins::Fragment_Id = 0;
        }

        output->fragment_number = 1;
        output->total_number_fragment = 1;
        output->byte_offset = 0;

        return output;
    }

    WARP_protocol* WARP_protocol::create_transmit(WARP_transmit_struct* info, uint8_t subtype) {
        uint8_t* buffer;
        uint8_t buffer_length;

        buffer_length = 2 + 2;
        buffer = (uint8_t*) std::malloc(buffer_length * sizeof(uint8_t));

        //Header
        buffer[TYPE_INDEX] = TYPE_TRANSMIT;
        buffer[SUBTYPE_INDEX] = subtype;

        //Transmit
        buffer[DATA_LENGTH_MSB_INDEX] = ((info->payload_size) >> 8) & 0xff;
        buffer[DATA_LENGTH_LSB_INDEX] = info->payload_size & 0xff;

        WARP_protocol* output = new WARP_protocol(buffer, buffer_length);
        std::free(buffer);
        return output;
    }

    WARP_protocol* WARP_protocol::create_mac_control(WARP_mac_control_struct* info) {
        uint8_t* buffer;
        uint8_t buffer_length = WARP_PROTOCOL_HEADER_LENGTH + MAC_CONTROL_ELEMENT_LENGTH;
        
        buffer = (uint8_t*) std::malloc(buffer_length * sizeof(uint8_t));

        //Header
        buffer[TYPE_INDEX] = TYPE_CONTROL;
        buffer[SUBTYPE_INDEX] = SUBTYPE_MAC_ADDRESS_CONTROL;

        //MAC control
        buffer[OPERTAION_CODE_INDEX] = info->operation_code;
        memcpy(buffer + OPERTAION_CODE_INDEX + 1, &(info->mac_address[0]), 6);

        WARP_protocol* output = new WARP_protocol(buffer, buffer_length);
        std::free(buffer);

        return output;
    }

    WARP_protocol* WARP_protocol::create_transmission_control(WARP_transmission_control_struct* info) {
        uint8_t* buffer;
        uint8_t buffer_length;

        buffer_length = WARP_PROTOCOL_HEADER_LENGTH + TRANSMISSION_CONTROL_ELEMENT_LENGTH;
        buffer = (uint8_t*) std::malloc(buffer_length * sizeof(uint8_t));

        //Header
        buffer[TYPE_INDEX] = TYPE_CONTROL;
        buffer[SUBTYPE_INDEX] = SUBTYPE_TRANSMISSION_CONTROL;
        buffer[NUM_ELEMENT_INDEX] = info->total_num_element;
        buffer[TRANSMISSION_OPERATION_CODE_INDEX] = info->operation_code;

        //Control
        memcpy(buffer + WARP_PROTOCOL_HEADER_LENGTH + 2, &(info->bssid[0]), 6);
        buffer[DISABLED_INDEX] = info->disabled;
        buffer[TX_POWER_INDEX] = info->tx_power;
        buffer[CHANNEL_INDEX] = info->channel;
        buffer[RATE_INDEX] = info->rate;
        buffer[HW_MODE_INDEX] = info->hw_mode;

        WARP_protocol* output = new WARP_protocol(buffer, buffer_length);
        std::free(buffer);

        return output;
    }

    /*********************************************************************************************************
                                        End of static methods
    *********************************************************************************************************/

    // WARP_protocol::WARP_protocol(const uint8_t *data, uint32_t total_sz) {
    //     // buffer = (uint8_t*) std::malloc(total_sz * 0 + 2048);
    //     //buffer = new uint8_t[total_sz];
    //     size = total_sz;
    //     for (uint32_t i = 0; i < total_sz; i++) {
    //         buffer[i] = data[i];
    //     }
    // }
    WARP_protocol::WARP_protocol(const uint8_t* data, uint32_t sz) : buffer(data, data + sz) { }
    
    WARP_protocol::~WARP_protocol() {
    }

    //Serialize entire packet (only assert if running in debug mode)
    void WARP_protocol::write_serialization(uint8_t *data, uint32_t sz, const PDU *parent) { 
        std::copy(buffer.begin(), buffer.end(), data);
    }
}