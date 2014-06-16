from scapy.all import *
import RadioTapHeader as HeaderBits

def generate(src = "00:11:22:33:44:55", dst = "02:00:00:000:00:00", show = False):
    radioTap = RadioTap()
    radioTap.len = 13
    radioTap.present = (1 << HeaderBits.RATE)
    radioTap.notdecoded = '\x02\x00\x00\x00\x00' #But what do they mean??

    element = Dot11()
    element.subtype = 11
    element.type = 0
    element.proto = 0
    element.addr1 = dst
    element.addr2 = src
    element.addr3 = dst
    element.SC = 0
    element.addr4 = None
    radioTap = radioTap / element

    element = Dot11Auth()
    element.algo = 1 #Open
    element.seqnum = 1 #From device to hostapd
    element.status = 'success'
    radioTap = radioTap / element

    if show:
        print "\nGenerated Authentication request:"
        radioTap.show()
        print "\n"
    return radioTap

if __name__ == "__main__":
    packet = generate()
    sendp(packet, iface = 'mon.wlan1', count = 2, inter = 0.1)
