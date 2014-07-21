#include <cstring>
#include <cassert>
#include "warp_transmit.h"
#include "tins/constants.h"
#include "tins/exceptions.h"
#include "tins/rawpdu.h"

namespace Tins {
    
    WARP_transmit::WARP_transmit(uint8_t power_param, uint8_t rate_param, uint8_t channel_param, uint8_t flag_param, uint8_t retry_param) {
        memset(&_WARP_transmit, 0, sizeof(WARP_transmit_struct)); //Clear Memory
        power(power_param);
        rate(rate_param);
        channel(channel_param);
        flag(flag_param);
        retry(retry_param);
    }
    
    WARP_transmit::WARP_transmit(const uint8_t *buffer, uint32_t total_sz) {
        if(total_sz > sizeof(WARP_transmit_struct))
            throw malformed_packet();
        memcpy(&_WARP_transmit, buffer, sizeof(WARP_transmit_struct)); //Copy Directly into Memory/Struct
        
        //If we have leftover buffer data, create a new RAW Packet and set to inner_pdu
        total_sz -= sizeof(WARP_transmit_struct);
        if(total_sz)
            inner_pdu(new RawPDU(buffer + sizeof(WARP_transmit_struct), total_sz));
    }
    
    //Setter for power
    void WARP_transmit::power(uint8_t new_power) {
        this->_WARP_transmit.power = new_power;
    }
    
    //Setter for rate
    void WARP_transmit::rate(uint8_t new_rate) {
        this->_WARP_transmit.rate = new_rate;
    }
    
    //Setter for channel
    void WARP_transmit::channel(uint8_t new_channel) {
        this->_WARP_transmit.channel = new_channel;
    }
    
    //Setter for flag
    void WARP_transmit::flag(uint8_t new_flag) {
        this->_WARP_transmit.flag = new_flag;
    }
    
    //Setter for retry
    void WARP_transmit::retry(uint8_t new_retry) {
        this->_WARP_transmit.retry = new_retry;
    }

    //Returns Header Size (Getter)
    uint32_t WARP_transmit::header_size() const {
        return sizeof(WARP_transmit_struct);
    }
    
    //Serialize entire packet (only assert if running in debug mode)
    void WARP_transmit::write_serialization(uint8_t *buffer, uint32_t total_sz, const PDU *parent) {
#ifdef TINS_DEBUG
        assert(total_sz >= sizeof(WARP_transmit_struct));
#endif
        memcpy(buffer, &_WARP_transmit, sizeof(WARP_transmit_struct));
    }
}