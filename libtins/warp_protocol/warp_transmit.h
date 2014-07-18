#ifndef WARP_TRANSMIT_PDU_H
#define WARP_TRANSMIT_PDU_H

#include "tins/macros.h"
#include "tins/pdu.h"
 
namespace Tins {
 
    class WARP_transmit: public PDU {
    public:
        
        static const PDU::PDUType pdu_flag = PDU::USER_DEFINED_PDU;

        //Constructor with Parameters (Default: power = 1, rate = 1, channel = 1, flag = 2
        WARP_transmit(uint8_t power_param = 0, uint8_t rate_param = 1, uint8_t channel_param = 1, uint8_t flag_param = 2, uint8_t retry = 0);
        
        //Constructor with buffer
        WARP_transmit(const uint8_t *buffer, uint32_t total_sz);
        
        //Getter for power
        uint8_t power() const { return _WARP_transmit.power; }
        
        //Getter for rate
        uint8_t rate() const { return _WARP_transmit.rate; }
        
        //Getter for channel
        uint8_t channel() const { return _WARP_transmit.channel; }
        
        //Getter for flag
        uint8_t flag() const { return _WARP_transmit.flag; }
        
        //Getter for retry
        uint8_t retry() const { return _WARP_transmit.retry; }


        //Setter for power
        void power(uint8_t new_power);
        
        //Setter for rate
        void rate(uint8_t new_rate);
        
        //Setter for channel
        void channel(uint8_t new_channel);
        
        //Setter for flag
        void flag(uint8_t new_flag);
        
        //Setter for retry
        void retry(uint8_t new_retry);

        //Returns PDU Type (Getter)
        PDUType pdu_type() const { return pdu_flag; }
        
        //Returns Header Size (Getter)
        uint32_t header_size() const;
        
        //Clones the PDU. This method is used when copying PDUs.
        WARP_transmit *clone() const { return new WARP_transmit(*this); }
        
    private:
        TINS_BEGIN_PACK
        struct WARP_transmit_struct {
            uint8_t power;
            uint8_t rate;
            uint8_t channel;
            uint8_t flag;
            uint8_t retry;
        } TINS_END_PACK;
        
        void write_serialization(uint8_t *buffer, uint32_t total_sz, const PDU *parent);
        
        WARP_transmit_struct _WARP_transmit;
    };
    
}
#endif