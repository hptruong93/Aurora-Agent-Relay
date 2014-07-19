#ifndef CONFIG_H
#define CONFIG_H

#include "tins/tins.h"

namespace Config {
    
    Tins::HWAddress<6> BROADCAST("ff:ff:ff:ff:ff:ff");
    Tins::HWAddress<6> HOSTAPD("00:21:5d:22:97:8c");
    Tins::HWAddress<6> PC_ENGINE("a0:a1:a2:a3:a4:a5"); //PCEngine --> hostapd
    Tins::HWAddress<6> WARP("d0:d1:d2:d3:d4:d5");

    static uint8_t default_radio_tap_buffer[14] = {0x00, 0x00, 0x0e, 0x00, 0x0e, 0x00, 0x00, 0x00, 0x00, 0x02, 0x71, 0x09, 0xa0, 0x00};
 
}

#endif
