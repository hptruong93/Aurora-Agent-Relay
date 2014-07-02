from scapy.all import *
import RadioTapHeader as HeaderBits
import config

def generate(src = config.CONFIG['general_mac']['WIFI_SRC'], dst = config.CONFIG['WARP_mac']['eth_a'], 
            ssid = config.CONFIG['hostapd']['ssid'], radio_tap_header = False, show = False):
    if radio_tap_header:
        radioTap = RadioTap()
        radioTap.len = 13 - 4 + 5
        radioTap.present = ((1 << HeaderBits.FLAGS) | (1 << HeaderBits.RATE) | (1 << HeaderBits.CHANNEL))
        radioTap.notdecoded = '\x00\x02q\t\xa0\x00'
    else:
        radioTap = None

    element = Dot11()
    element.subtype = 0 #Association request
    element.type = 0
    element.proto = 0
    element.addr1 = dst
    element.addr2 = src
    element.addr3 = dst
    element.SC = 704
    element.addr4 = None
    if radioTap is not None:
        radioTap = radioTap / element
    else:
        radioTap = element

    element = Dot11AssoReq()
    element.cap = ((1 << 2) | (1 << 8) | (1 << 12) | (1 << 13)) #short-slot, ESS, privacy, short-preamble
    element.listen_interval = 10
    radioTap = radioTap / element

    element = Dot11Elt()
    element.ID = 'SSID'
    element.len = len(ssid)
    element.info = ssid
    radioTap = radioTap / element
    
    element = Dot11Elt()
    element.ID = 'Rates'
    element.len = 8
    element.info = '\x02\x04\x0b\x16\x0c\x12\x18$' #But what do they mean?
    radioTap = radioTap / element
    
    element = Dot11Elt()
    element.ID = 'ESRates'
    element.len = 4
    element.info = '0H`l' #But what do they mean?
    radioTap = radioTap / element

    element = Dot11Elt()
    element.ID = 'vendor'
    element.len = 7
    element.info = '\x00P\xf2\x02\x00\x01\x00' #But what do they mean?
    radioTap = radioTap / element


    if show:
        print "\nGenerated Authentication request:"
        radioTap.show()
        print "\n"
    return radioTap

if __name__ == "__main__":
    packet = generate(src = config.CONFIG['PC_mac']['WLAN0'], radio_tap_header = True, show = True)
    sendp(packet, iface = 'mon0', count = 1, inter = 0.05)
