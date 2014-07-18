#ifndef WARP_CONTROL_PDU_H
#define WARP_CONTROL_PDU_H

#include "tins/macros.h"
#include "tins/pdu.h"
 
namespace Tins {
 
    class WARP_MAC_control: public PDU {
    public:
        
        static const PDU::PDUType pdu_flag = PDU::PDUType::USER_DEFINED_PDU;

        //Constructor with Parameters (Default: power = 1, rate = 1, channel = 1, flag = 2
        WARP_MAC_control(uint8_t power_param = 0, uint8_t rate_param = 1, uint8_t channel_param = 1, uint8_t flag_param = 2);
        
        //Constructor with buffer
        WARP_MAC_control(const uint8_t *buffer, uint32_t total_sz);
        
        //Getter for operation
        uint8_t operation() const { return _WARP_mac_control.operation; }
        
        //Getter for mac_address
        HWAddress<6> mac_address() const { return _WARP_mac_control.mac_address; }

        //Setter for operation
        void operation(uint8_t new_operation);
        
        //Setter for mac_address
        void mac_address(HWAddress<6> new_mac_address);

        //Returns PDU Type (Getter)
        PDUType pdu_type() const { return pdu_flag; }
        
        //Returns Header Size (Getter)
        uint32_t header_size() const;
        
        //Clones the PDU. This method is used when copying PDUs.
        WARP_MAC_control *clone() const { return new WARP_MAC_control(*this); }
        
    private:
        TINS_BEGIN_PACK
        struct WARP_MAC_control_struct {
            uint8_t operation;
            HWAddress<6> mac_address;
        } TINS_END_PACK;
        
        void write_serialization(uint8_t *buffer, uint32_t total_sz, const PDU *parent);
        
        WARP_MAC_control_struct _WARP_mac_control;
    };
    
}
#endif