#include "tins/tins.h"
#include "config.h"

namespace Config {
    const uint16_t MAX_ETHERNET_LENGTH = 1475; //Actually 1500 but reserve 25 bytes for future development

    Tins::HWAddress<6> BROADCAST("ff:ff:ff:ff:ff:ff");
    Tins::HWAddress<6> HOSTAPD("40:d8:55:04:22:84");
    Tins::HWAddress<6> PC_ENGINE("00:0D:B9:34:17:29");
    Tins::HWAddress<6> WARP("40:d8:55:04:22:85");
    Tins::HWAddress<6> DEFAULT_MAC("00:00:00:00:00:00");

    const uint8_t default_radio_tap_buffer[14] = {0x00, 0x00, 0x0e, 0x00, 0x0e, 0x00, 0x00, 0x00, 0x00, 0x02, 0x71, 0x09, 0xa0, 0x00};
}