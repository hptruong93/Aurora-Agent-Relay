from scapy.all import *
import RadioTapHeader as HeaderBits

def generate(src = "00:11:22:33:44:55", dst = "ff:ff:ff:ff:ff:ff", show = False):
    radioTap = RadioTap()
    radioTap.len = 18
    radioTap.present = ((1 << HeaderBits.FLAGS) | (1 << HeaderBits.RATE) | (1 << HeaderBits.CHANNEL))
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

if __name__ == "__main__":
    packet = generate()
    sendp(packet, iface = 'mon.wlan1', count = 2, inter = 0.1)
