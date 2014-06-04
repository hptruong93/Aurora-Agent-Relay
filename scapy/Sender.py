from scapy.all import *

class Sender:

    DEFAULT_DST = "00:22:15:76:67:80"
    BROADCAST = "ff:ff:ff:ff:ff:ff"
    DEFAULT_VWFACE = "wlan0"
    HEADER = "\x00\x00\x0e\x00\x0e\x00\x00\x00\x00\x02\x80\t\xa0\x00"

    def __init__(self):
        print "init sniffer"

    def sniffing(self):
        sniff(iface="eth0", prn=lambda x: self._process(x))

    def _process(self, pkt):
        print pkt.dst
        if pkt.dst == self.BROADCAST:
            dot11_frame = RadioTap(self.HEADER) / Dot11(str(pkt.payload))
#            print dot11_frame.show()
            sendp(dot11_frame, iface=self.DEFAULT_VWFACE)


if __name__ == '__main__':
    sender = Sender()
    sender.sniffing()
