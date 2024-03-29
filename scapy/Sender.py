from scapy.all import *

class WARPControlHeader(Packet):
    name = "WARPControlHeader"
    fields_desc = [ XByteField("power", 0),
                   XByteField("rate", 1),
                   XByteField("channel", 1),
                   XByteField("flag", 2)
                 ]

class Sender:

    DEFAULT_DST = "c0:c1:c2:c3:c4:c5"
    BROADCAST = "ff:ff:ff:ff:ff:ff"
    DEFAULT_VWFACE = "mon.wlan0"

    def __init__(self):
        print "init sniffer"

    def sniffing(self):
        sniff(iface="eth4", prn=lambda x: self._process(x))

    def _process(self, pkt):
        if pkt.dst == self.BROADCAST or pkt.dst == self.DEFAULT_DST:
            Wpacket = WARPControlHeader(str(pkt.payload))
            dot11_frame = Dot11(str(Wpacket.payload))
            sendp(dot11_frame, iface=self.DEFAULT_VWFACE)

if __name__ == '__main__':
    sender = Sender()
    sender.sniffing()
