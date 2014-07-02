from scapy.all import *
import RadioTapHeader as HeaderBits
import config

def generate(src = config.CONFIG['general_mac']['WIFI_SRC'], dst = config.CONFIG['WARP_mac']['eth_a'], radio_tap_header = False, show = False):
    if radio_tap_header:
        radioTap = RadioTap()
        radioTap.len = 14
        radioTap.present = ((1 << HeaderBits.FLAGS) | (1 << HeaderBits.RATE) | (1 << HeaderBits.CHANNEL))
        radioTap.notdecoded = '\x00\x02q\t\xa0\x00'
    else:
        radioTap = None

    element = Dot11()
    element.subtype = 11
    element.type = 0
    element.proto = 0
    element.addr1 = dst
    element.addr2 = src
    element.addr3 = dst
    element.SC = 0
    element.addr4 = None
    if radioTap:
        radioTap = radioTap / element
    else:
        radioTap = element

    element = Dot11Auth()
    element.algo = 0 #Open
    element.seqnum = 1 #From device to hostapd
    element.status = 'success'
    radioTap = radioTap / element

    if show:
        print "\nGenerated Authentication request:"
        radioTap.show()
        print "\n"
    return radioTap

if __name__ == "__main__":
    packet = generate(src = config.CONFIG['PC_mac']['WLAN0'], radio_tap_header = True, show = True)
    sendp(packet, iface = 'mon0', count = 1, inter = 0.05)
