#ifndef CONFIG_H_
#define CONFIG_H_

#include "tins/tins.h"

#define GREP_FROM_IFCONFIG			"ifconfig | grep -E "
#define HW_ADDR_KEYWORD				"HWaddr "


namespace Config {
    extern const uint16_t MAX_ETHERNET_LENGTH; //Actually 1500 but reserve 25 bytes for future development

    extern Tins::HWAddress<6> BROADCAST;
    extern Tins::HWAddress<6> HOSTAPD;//"00:21:5d:22:97:8c");
    extern Tins::HWAddress<6> PC_ENGINE;
    extern Tins::HWAddress<6> WARP;
    extern Tins::HWAddress<6> DEFAULT_MAC;

    extern const uint8_t default_radio_tap_buffer[14];
}

#endif
