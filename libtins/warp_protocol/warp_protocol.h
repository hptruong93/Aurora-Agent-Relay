#ifndef WARP_PROTOCOL_H
#define WARP_PROTOCOL_H

#include "tins/macros.h"
#include "tins/pdu.h"
#include "tins/tins.h"
#include "../revised_version/config.h"

#define WARP_PROTOCOL_TYPE                  0x8ae
#define WARP_PROTOCOL_MAX_SIZE                        100
#define WARP_PROTOCOL_HEADER_LENGTH                   2

//Length of elements
#define TRANSMIT_ELEMENT_LENGTH                       7
#define MAC_CONTROL_ELEMENT_LENGTH                    7
#define TRANSMISSION_CONTROL_ELEMENT_LENGTH           11        

#define TYPE_INDEX                                    0
#define SUBTYPE_INDEX                                 1

#define TYPE_IGNORE                                   0
#define TYPE_TRANSMIT                                 1
#define TYPE_CONTROL                                  2

//For type = transmit
#define SUBTYPE_MANGEMENT_TRANSMIT                    0
#define SUBTYPE_DATA_TRANSMIT                         1

#define BSSID_INDEX                                   2 //6 bytes following will be bssid
#define FLAG_INDEX                                    8
#define RETRY_INDEX                                   9
#define DATA_LENGTH_MSB_INDEX                         10
#define DATA_LENGTH_LSB_INDEX                         11

//For fragment info
#define FRAGMENT_INFO_INDEX                           12
#define FRAGMENT_INFO_LENGTH                          5
#define FRAGMENT_ID_INDEX                             0
#define FRAGMENT_NUMBER_INDEX                         1
#define FRAGMENT_TOTAL_NUMBER_INDEX                   2
#define FRAGMENT_BYTE_OFFSET_MSB                      3
#define FRAGMENT_BYTE_OFFSET_LSB                      4

//For type = control
#define SUBTYPE_TRANSMISSION_CONTROL                  1
#define SUBTYPE_MAC_ADDRESS_CONTROL                   2
#define DISABLED_INDEX                                8
#define TX_POWER_INDEX                                9
#define CHANNEL_INDEX                                 10
#define RATE_INDEX                                    11
#define HW_MODE_INDEX                                 12


#define DEFAULT_TRANSMIT_POWER                        0
#define DEFAULT_TRANSMIT_RATE                         1
#define DEFAULT_TRANSMIT_CHANNEL                      1
#define DEFAULT_TRANSMIT_FLAG                         0
#define DEFAULT_TRANSMIT_RETRY                        0
#define MAX_RETRY                                     7
#define DEFAULT_MAC_CONTROL_OPERATION_CODE            0
#define DEFAULT_TRANSMISSION_CONTROL_DISABLED         0
#define DEFAULT_TRANSMISSION_CONTROL_TX_POWER         0
#define DEFAULT_TRANSMISSION_CONTROL_CHANNEL          2
#define DEFAULT_TRANSMISSION_CONTROL_RATE             1
#define DEFAULT_TRANSMISSION_CONTROL_HW_MODE          0

namespace Tins {
 
    class WARP_protocol: public PDU {

    public:
        
        typedef struct WARP_transmit {
            uint8_t flag;
            uint8_t retry;
            uint16_t payload_size;
            uint8_t bssid[6];
        } WARP_transmit_struct;
        
        typedef struct WARP_fragment {
            uint8_t id;
            uint8_t fragment_number;
            uint8_t total_number_fragment;
            uint16_t byte_offset;
            uint32_t length;
        } WARP_fragment_struct;

        struct WARP_mac_control_struct {
            uint8_t operation_code;
            uint8_t mac_address[6];
        };

        struct WARP_transmission_control_struct {
            uint8_t bssid[6];
            uint8_t disabled;
            uint8_t tx_power;
            uint8_t channel;
            uint8_t rate;
            uint8_t hw_mode;
        };

        static uint16_t fragment_id = 0;
        
        static const PDU::PDUType pdu_flag = PDU::USER_DEFINED_PDU;

        static uint32_t process_warp_layer(uint8_t* input_buffer, WARP_transmit_struct* transmit_result);

        static WARP_transmit_struct* get_default_transmit_struct(Tins::HWAddress<6> bssid = Config::HOSTAPD);

        static WARP_mac_control_struct* get_default_mac_control_struct(Tins::HWAddress<6> mac_address = Config::DEFAULT_MAC);

        static WARP_transmission_control_struct* get_default_transmission_control_struct(Tins::HWAddress<6> bssid = Config::HOSTAPD);

        static WARP_fragment_struct* generate_fragment_struct();

        static WARP_protocol* create_transmit(WARP_transmit_struct* info, WARP_fragment_struct* fragment_info, uint8_t subtype);

        static WARP_protocol* create_mac_control(WARP_mac_control_struct* info);

        static WARP_protocol* create_transmission_control(WARP_transmission_control_struct* info);

        //Constructor with buffer
        WARP_protocol(const uint8_t *data, uint32_t total_sz);
        
        ~WARP_protocol();

        void free_buffer();

        //Returns PDU Type (Getter)
        PDUType pdu_type() const { return pdu_flag; }
        
        //Returns Header Size (Getter)
        // uint32_t header_size() const { return size; }
        uint32_t header_size() const { return buffer.size(); }
        
        //Clones the PDU. This method is used when copying PDUs.
        WARP_protocol *clone() const { return new WARP_protocol(*this); }
        
        void write_serialization(uint8_t *data, uint32_t total_sz, const PDU *parent);
    
        uint8_t* get_buffer() {
            // return buffer;
            return &(buffer[0]);
        }

    private:
        // uint8_t buffer[2048];
        std::vector<uint8_t> buffer;
        uint32_t size;
    };
    
}
#endif
