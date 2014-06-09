from scapy.all import *
import packet_util

_FLAGS = 1
_RATE = 2
_CHANNEL = 3
_ANTENNA_SIGNAL = 5
_ANTENNA_NOISE = 6
_ANTENNA = 11
_DB_ANTENNA_SIGNAL = 12
_DB_ANTENNA_NOISE = 13
_RX_FLAG = 14

def generate(src = "00:11:22:33:44:55", dst = "ff:ff:ff:ff:ff:ff", show = False):
    radioTap = RadioTap()
    radioTap.len = 18
    radioTap.present = ((1 << _FLAGS) | (1 << _RATE) | (1 << _CHANNEL))
    radioTap.notdecoded = '\x00\x02q\t\xa0\x00\xd7\x02\x00\x00' #But what do they mean??
    radioTap = radioTap / Dot11ProbeReq()

    element = Dot11()
    element.subtype = 4
    element.type = 0
    element.proto = 0
    element.addr1 = dst
    element.addr2 = src
    element.addr3 = "ff:ff:ff:ff:ff:ff"
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