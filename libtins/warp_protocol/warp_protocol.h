#ifndef WARP_TRANSMIT_PDU_H
#define WARP_TRANSMIT_PDU_H

#include "tins/macros.h"
#include "tins/pdu.h"
 
#define WARP_PROTOCOL_HEADER_LENGTH                   2

namespace Tins {
 
    class WARP_protocol: public PDU {
    public:
        
        static const PDU::PDUType pdu_flag = PDU::PDUType::USER_DEFINED_PDU;

        //Constructor with buffer
        WARP_protocol(const uint8_t *buffer, uint32_t total_sz);
        
        //Returns PDU Type (Getter)
        PDUType pdu_type() const { return pdu_flag; }
        
        //Returns Header Size (Getter)
        uint32_t header_size() const { return buffer.size(); }
        
        //Clones the PDU. This method is used when copying PDUs.
        WARP_protocol *clone() const { return new WARP_protocol(*this); }
        
        void write_serialization(uint8_t *buffer, uint32_t total_sz, const PDU *parent);

        const std::vector<uint8_t> &get_buffer() const {
            return buffer;
        }
        
    private:
        std::vector<uint8_t> buffer;
    };
    
}
#endif
