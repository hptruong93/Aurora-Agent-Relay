from scapy.all import *

class WARPControlHeader(Packet):
	name = "WARPControlHeader"
	fields_desc = [ XByteField("power", 0),
                   XByteField("rate", 1),
                   XByteField("channel", 1),
                   XByteField("flag", 2)
                 ]

class Sniffer:

    DEFAULT_SRC = "c0:c1:c2:c3:c4:c5"
    DEFAULT_DST = "a0:a1:a2:a3:a4:a5"
    DEFAULT_IFACE = "eth4"
    FILTER = "02:00:00:00:00:00"

    def __init__(self):
        print "init sniffer"

    def sniffing(self):
        sniff(iface="hwsim0", prn=lambda x: self._process(x))

    def _process(self, pkt):
        if pkt.payload.addr2 == self.FILTER or pkt.payload.addr2 == "ff:ff:ff:ff:ff:ff":
            eth_frame = Ether() / WARPControlHeader() / pkt.payload
            eth_frame.show()
            eth_frame.src = self.DEFAULT_SRC
            eth_frame.dst = self.DEFAULT_DST
            sendp(eth_frame, iface = self.DEFAULT_IFACE)

if __name__ == '__main__':
    sniffer = Sniffer()
    sniffer.sniffing()
