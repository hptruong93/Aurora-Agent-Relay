#ifndef WARP_PROTOCOL_H
#define WARP_PROTOCOL_H

#include "tins/macros.h"
#include "tins/pdu.h"
#include "tins/tins.h"
#include "../revised_version/config.h"

#define WARP_PROTOCOL_TYPE                  0x8ae
#define WARP_PROTOCOL_MAX_SIZE                        100
#define WARP_PROTOCOL_HEADER_LENGTH                   2

#define TYPE_INDEX                                    0
#define SUBTYPE_INDEX                                 1

#define TYPE_TRANSMIT                                 1
#define TYPE_CONTROL                                  2

//For type = transmit
#define SUBTYPE_MANGEMENT_TRANSMIT                    0
#define SUBTYPE_DATA_TRANSMIT                         1

#define BSSID_INDEX                                   2 //6 bytes following will be bssid
#define FLAG_INDEX                                    8
#define RETRY_INDEX                                   9
#define DATA_LENGTH_MSB                               10
#define DATA_LENGTH_LSB                               11
#define FRAGMENT_INFO_INDEX                           12

//For fragment info
#define FRAGMENT_INFO_LENGTH                          5
#define FRAGMENT_ID_INDEX                             0
#define FRAGMENT_NUMBER_INDEX                         1
#define FRAGMENT_TOTAL_NUMBER_INDEX                   2
#define FRAGMENT_BYTE_OFFSET_MSB                      3
#define FRAGMENT_BYTE_OFFSET_LSB                      4

//For type = control
#define TRANSMIT_ELEMENT_LENGTH                       7
#define MAC_CONTROL_ELEMENT_LENGTH                    7

//For type = control
#define SUBTYPE_TRANSMISSION_CONTROL                  1
#define SUBTYPE_MAC_ADDRESS_CONTROL                   2


#define DEFAULT_TRANSMIT_POWER                        0
#define DEFAULT_TRANSMIT_RATE                         1
#define DEFAULT_TRANSMIT_CHANNEL                      1
#define DEFAULT_TRANSMIT_FLAG                         0
#define DEFAULT_TRANSMIT_RETRY                        0
#define MAX_RETRY                                     7


namespace Tins {
 
    class WARP_protocol: public PDU {

    public:
        
        struct WARP_transmit_struct {
            uint8_t flag;
            uint8_t retry;
            uint16_t payload_size;
            uint8_t bssid[6];
        };
        
        struct WARP_fragment_struct {
            uint8_t id;
            uint8_t fragment_number;
            uint8_t total_number_fragment;
            uint16_t byte_offset;
            uint32_t length;
        };
        
        static const PDU::PDUType pdu_flag = PDU::USER_DEFINED_PDU;

        static uint32_t process_warp_layer(uint8_t* input_buffer);

        static WARP_transmit_struct* get_default_transmit_struct(Tins::HWAddress<6> bssid = Config::HOSTAPD);

        static WARP_fragment_struct* generate_fragment_struct();

        static WARP_protocol* create_transmit(WARP_transmit_struct* info, WARP_fragment_struct* fragment_info, uint8_t subtype);

        static WARP_protocol* create_mac_control(uint8_t operation_code, uint8_t* mac_address);

        //Constructor with buffer
        WARP_protocol(const uint8_t *data, uint32_t total_sz);
        
        ~WARP_protocol();

        //Returns PDU Type (Getter)
        PDUType pdu_type() const { return pdu_flag; }
        
        //Returns Header Size (Getter)
        uint32_t header_size() const { return size; }
        
        //Clones the PDU. This method is used when copying PDUs.
        WARP_protocol *clone() const { return new WARP_protocol(*this); }
        
        void write_serialization(uint8_t *data, uint32_t total_sz, const PDU *parent);
    
        uint8_t* get_buffer() {
            return buffer;
        }

    private:
        uint8_t* buffer;
        uint32_t size;
    };
    
}
#endif
