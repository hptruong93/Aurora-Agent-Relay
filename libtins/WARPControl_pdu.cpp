/* WARPControl_pdu.cpp: Protocol for WARP Control Frame
 * Kevin Han
 * McGill University Broadband Communications Research Lab
 * Smart Applications on Virtual Infrastructure Team
 * May 2014 */

#include <cstring>
#include <cassert>
#include "WARPControl_pdu.h"
#include "tins/constants.h"
#include "tins/exceptions.h"
#include "tins/rawpdu.h"

namespace Tins {
    
    WARPControlPDU::WARPControlPDU(uint8_t power_param, uint8_t rate_param, uint8_t channel_param, uint8_t flag_param) {
        memset(&_WARPControl, 0, sizeof(WARPControlStruct)); //Clear Memory
        power(power_param);
        rate(rate_param);
        channel(channel_param);
        flag(flag_param);
    }
    
    WARPControlPDU::WARPControlPDU(const uint8_t *buffer, uint32_t total_sz) {
        if(total_sz > sizeof(WARPControlStruct))
            throw malformed_packet();
        memcpy(&_WARPControl, buffer, sizeof(WARPControlStruct)); //Copy Directly into Memory/Struct
        
        //If we have leftover buffer data, create a new RAW Packet and set to inner_pdu
        total_sz -= sizeof(WARPControlStruct);
        if(total_sz)
            inner_pdu(new RawPDU(buffer + sizeof(WARPControlStruct), total_sz));
    }
    
    //Setter for power
    void WARPControlPDU::power(uint8_t new_power) {
        this->_WARPControl.WARPpower = new_power;
    }
    
    //Setter for rate
    void WARPControlPDU::rate(uint8_t new_rate) {
        this->_WARPControl.WARPrate = new_rate;
    }
    
    //Setter for channel
    void WARPControlPDU::channel(uint8_t new_channel) {
        this->_WARPControl.WARPchannel = new_channel;
    }
    
    //Setter for flag
    void WARPControlPDU::flag(uint8_t new_flag) {
        this->_WARPControl.WARPflag = new_flag;
    }
    
    //Returns Header Size (Getter)
    uint32_t WARPControlPDU::header_size() const {
        return sizeof(WARPControlStruct);
    }
    
    //Serialize entire packet (only assert if running in debug mode)
    void WARPControlPDU::write_serialization(uint8_t *buffer, uint32_t total_sz, const PDU *parent) {
        #ifdef TINS_DEBUG
        assert(total_sz >= sizeof(WARPControlStruct));
        #endif
        memcpy(buffer, &_WARPControl, sizeof(WARPControlStruct));
    }
    
}
