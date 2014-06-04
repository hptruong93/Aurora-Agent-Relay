/* WARPControl_pdu.h: Protocol for WARP Control Frame
 * Kevin Han
 * McGill University Broadband Communications Research Lab
 * Smart Applications on Virtual Infrastructure Team
 * May 2014 */
 
#ifndef WARP_CONTROL_PDU_H
#define WARP_CONTROL_PDU_H 

#include "tins/macros.h"
#include "tins/pdu.h"
 
namespace Tins {
 
    class WARPControlPDU: public PDU {
    public:
        
        static const PDU::PDUType pdu_flag = PDU::PDUType::USER_DEFINED_PDU;

        //Constructor with Parameters (Default: power = 1, rate = 1, channel = 1, flag = 2
        WARPControlPDU(uint8_t power_param = 0, uint8_t rate_param = 1, uint8_t channel_param = 1, uint8_t flag_param = 2);
        
        //Constructor with buffer
        WARPControlPDU(const uint8_t *buffer, uint32_t total_sz);
        
        //Getter for power
        uint8_t power() const { return _WARPControl.WARPpower; }
        
        //Getter for rate
        uint8_t rate() const { return _WARPControl.WARPrate; }
        
        //Getter for channel
        uint8_t channel() const { return _WARPControl.WARPchannel; }
        
        //Getter for flag
        uint8_t flag() const { return _WARPControl.WARPflag; }
        
        //Setter for power
        void power(uint8_t new_power);
        
        //Setter for rate
        void rate(uint8_t new_rate);
        
        //Setter for channel
        void channel(uint8_t new_channel);
        
        //Setter for flag
        void flag(uint8_t new_flag);
        
        //Returns PDU Type (Getter)
        PDUType pdu_type() const { return pdu_flag; }
        
        //Returns Header Size (Getter)
        uint32_t header_size() const;
        
        //Clones the PDU. This method is used when copying PDUs.
        WARPControlPDU *clone() const { return new WARPControlPDU(*this); }
        
    private:
        TINS_BEGIN_PACK
        struct WARPControlStruct {
            uint8_t WARPpower;
            uint8_t WARPrate;
            uint8_t WARPchannel;
            uint8_t WARPflag;
        } TINS_END_PACK;
        
        void write_serialization(uint8_t *buffer, uint32_t total_sz, const PDU *parent);
        
        WARPControlStruct _WARPControl;
    };
    
}
#endif
