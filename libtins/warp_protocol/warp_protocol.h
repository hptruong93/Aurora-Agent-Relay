#ifndef WARP_PROTOCOL_H
#define WARP_PROTOCOL_H

#include <mutex>

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
#define BSSID_CONTROL_ELEMENT_LENGTH                  8         // This is the length WITHOUT mac addresses!!!       

#define TYPE_INDEX                                    0
#define SUBTYPE_INDEX                                 1

#define TYPE_IGNORE                                   0
#define TYPE_TRANSMIT                                 1
#define TYPE_CONTROL                                  2

#define HEADER_OFFSET                                 2
//For type = transmit
#define SUBTYPE_MANGEMENT_TRANSMIT                    0
#define SUBTYPE_DATA_TRANSMIT                         1

#define DATA_LENGTH_MSB_INDEX                         2
#define DATA_LENGTH_LSB_INDEX                         3

//For fragment info
#define FRAGMENT_INFO_INDEX                           12
#define FRAGMENT_INFO_LENGTH                          5
#define FRAGMENT_ID_INDEX                             0
#define FRAGMENT_NUMBER_INDEX                         1
#define FRAGMENT_TOTAL_NUMBER_INDEX                   2
#define FRAGMENT_BYTE_OFFSET_MSB                      3
#define FRAGMENT_BYTE_OFFSET_LSB                      4

//For type = control
#define SUBTYPE_MAC_ADDRESS_CONTROL                   4
#define SUBTYPE_TRANSMISSION_CONTROL                  8
#define SUBTYPE_BSSID_CONTROL                         12

// For subtype = transmission control
#define TRANSMISSION_CONFIGURE_CODE                   1
#define TRANSMISSION_VERIFY_CODE                      4
#define TRANSMISSION_VERIFIED_CODE                    5
#define TRANSMISSION_INCONSISTENT_CODE                6
#define TRANSMISSION_CONFIGURE_SUCCESS_CODE           8
#define TRANSMISSION_CONFIGURE_FAIL_CODE              16

#define TRANSMISSION_NUM_ELEMENT_INDEX                2
#define TRANSMISSION_OPERATION_CODE_INDEX             3
#define TRANSMISSION_DISABLED_INDEX                   10
#define TRANSMISSION_TX_POWER_INDEX                   11
#define TRANSMISSION_CHANNEL_INDEX                    12
#define TRANSMISSION_RATE_INDEX                       13
#define TRANSMISSION_HW_MODE_INDEX                    14

// For subtype = mac adress control
#define NOTHING_CODE                                  0
#define MAC_ADD_CODE                                  1
#define MAC_REMOVE_CODE                               32
#define MAC_CLEAR_CODE                                33
#define MAC_CHECK_IF_EXIST_CODE                       64
#define MAC_EXISTED_CODE                              65
#define MAC_NOT_EXISTED_CODE                          66

#define OPERTAION_CODE_INDEX                          2

// For subtype = bssid control
#define BSSID_STATION_ASSOCIATE_CODE                  1
#define BSSID_STATION_DISASSOCIATE_CODE               32
#define BSSID_STATION_EXISTED_CODE                    65
#define BSSID_STATION_NOT_EXISTED_CODE                66
#define BSSID_STATION_CLEAR_CODE                      33

#define BSSID_NUM_ELEMENT_INDEX                       2
#define BSSID_BSSID_INDEX                             3
#define BSSID_OPERATION_CODE_INDEX                    9
#define BSSID_STATION_MAC_ADDR_INDEX                  10

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
#define DEFAULT_TRNASMISSION_CONTROL_OPERATION_CODE   0

namespace Tins {

    static uint16_t Fragment_Id = 0;
    static std::mutex Fragment_Id_Mux;
 
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
            uint8_t total_num_element;
            uint8_t bssid[6];
            uint8_t disabled;
            uint8_t tx_power;
            uint8_t channel;
            uint8_t rate;
            uint8_t hw_mode;
            uint8_t operation_code;
        };

        struct WARP_bssid_control_struct {
            uint8_t total_num_element;
            uint8_t bssid[6];
            uint8_t operation_code;
            uint8_t (*mac_addr)[6];
        };
        
        static const PDU::PDUType pdu_flag = PDU::USER_DEFINED_PDU;

        static uint8_t check_warp_layer_type(uint8_t* input_buffer);

        static uint32_t process_warp_layer(uint8_t* input_buffer, void* transmit_result);

        static WARP_transmit_struct* get_default_transmit_struct(Tins::HWAddress<6> bssid = Config::HOSTAPD);

        static WARP_mac_control_struct* get_default_mac_control_struct(Tins::HWAddress<6> mac_address = Config::DEFAULT_MAC);

        static WARP_transmission_control_struct* get_default_transmission_control_struct(Tins::HWAddress<6> bssid = Config::HOSTAPD);

        static WARP_fragment_struct* generate_fragment_struct();

        static WARP_protocol* create_transmit(WARP_transmit_struct* info, uint8_t subtype);

        static WARP_protocol* create_mac_control(WARP_mac_control_struct* info);

        static WARP_protocol* create_transmission_control(WARP_transmission_control_struct* info);

        static WARP_protocol* create_bssid_control(WARP_bssid_control_struct* info);

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
