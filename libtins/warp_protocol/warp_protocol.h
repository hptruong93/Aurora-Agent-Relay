#ifndef WARP_TRANSMIT_PDU_H
#define WARP_TRANSMIT_PDU_H

#include "tins/macros.h"
#include "tins/pdu.h"
#include "tins/tins.h"
 
#define WARP_PROTOCOL_HEADER_LENGTH                   2

#define TYPE_INDEX                                    0
#define SUBTYPE_INDEX                                 1

#define TYPE_TRANSMIT                                 1
#define TYPE_CONTROL                                  2

#define TRANSMIT_ELEMENT_LENGTH                       7
#define MAC_CONTROL_ELEMENT_LENGTH                    7

#define SUBTYPE_TRANSMISSION_CONTROL                  1
#define SUBTYPE_MAC_ADDRESS_CONTROL                   2


#define DEFAULT_TRANSMIT_POWER                        0
#define DEFAULT_TRANSMIT_RATE                         1
#define DEFAULT_TRANSMIT_CHANNEL                      1
#define DEFAULT_TRANSMIT_FLAG                         0
#define DEFAULT_TRANSMIT_RETRY                        0




namespace Tins {
 
    class WARP_protocol: public PDU {

    public:
        
        struct WARP_transmit_struct {
            uint8_t power;
            uint8_t rate;
            uint8_t channel;
            uint8_t flag;
            uint8_t retry;
            uint16_t payload_size;
        };
        
        static const PDU::PDUType pdu_flag = PDU::USER_DEFINED_PDU;

        static uint32_t process_warp_layer(uint8_t* input_buffer);

        static WARP_protocol* create_transmit(WARP_transmit_struct* info, uint8_t subtype);

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
