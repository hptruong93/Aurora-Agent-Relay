from scapy.all import *
import RadioTapHeader as HeaderBits

def generate(src = "00:11:22:33:44:55", dst = "02:00:00:000:00:00", ssid = 'test', show = False):
    radioTap = RadioTap()
    radioTap.len = 13
    radioTap.present = (1 << HeaderBits.RATE)
    radioTap.notdecoded = '\x02\x00\x00\x00\x00' #But what do they mean??

    element = Dot11()
    element.subtype = 0 #Association request
    element.type = 0
    element.proto = 0
    element.addr1 = dst
    element.addr2 = src
    element.addr3 = dst
    element.SC = 704
    element.addr4 = None
    radioTap = radioTap / element

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
    packet = generate()
    sendp(packet, iface = 'mon.wlan1', count = 2, inter = 0.1)
