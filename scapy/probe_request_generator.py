from scapy.all import *
import RadioTapHeader as HeaderBits
import config

def generate(src = config.CONFIG['general_mac']['WIFI_SRC'], dst = config.CONFIG['general_mac']['BROADCAST'], radio_tap_header = False, show = False):
    if radio_tap_header:
        radioTap = RadioTap()
        radioTap.len = 14
        radioTap.present = ((1 << HeaderBits.FLAGS) | (1 << HeaderBits.RATE) | (1 << HeaderBits.CHANNEL))
        radioTap.notdecoded = '\x00\x02q\t\xa0\x00' #Flag = 0; Rate = 2 x 500Kbps; Channel frequency = 0x7174, Channel flag = 0xa000
        radioTap = radioTap / Dot11ProbeReq()
    else:
        radioTap = Dot11ProbeReq()

    element = Dot11()
    element.subtype = 4
    element.type = 0
    element.proto = 0
    element.addr1 = dst
    element.addr2 = src
    element.addr3 = config.CONFIG['general_mac']['BROADCAST']
    element.SC = 13456
    element.addr4 = "00:00:00:00:00"
    radioTap = radioTap / element

    element = Dot11Elt()
    element.ID = 'SSID'
    element.len = 0
    element.info = ''
    radioTap = radioTap / element

    element = Dot11Elt()
    element.ID = 'Rates'
    element.len = 8
    element.info = '\x82\x84\x8b\x96\x0c\x12\x18$' #But what do they mean?
    radioTap = radioTap / element

    if show:
        print "\nGenerated Probe request:"
        radioTap.show()
        print "\n"
    return radioTap

if __name__ == "__main__":
    packet = generate(src = config.CONFIG['PC_mac']['WLAN0'], radio_tap_header = True, show = True)
    sendp(packet, iface = 'mon0', count = 1, inter = 0.1)
