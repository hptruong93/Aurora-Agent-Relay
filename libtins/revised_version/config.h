#ifndef CONFIG_H
#define CONFIG_H

#include "tins/tins.h"

#define WARP_PROTOCOL_TYPE                  0x8ae
#define MAX_RETRY							7

namespace Config {
    
    Tins::HWAddress<6> BROADCAST("ff:ff:ff:ff:ff:ff");
    Tins::HWAddress<6> HOSTAPD("40:d8:55:04:22:84");//"00:21:5d:22:97:8c");
    Tins::HWAddress<6> PC_ENGINE("00:0D:B9:34:17:29");
    Tins::HWAddress<6> WARP("40:d8:55:04:22:85");

    static uint8_t default_radio_tap_buffer[14] = {0x00, 0x00, 0x0e, 0x00, 0x0e, 0x00, 0x00, 0x00, 0x00, 0x02, 0x71, 0x09, 0xa0, 0x00};
 
}

#endif
