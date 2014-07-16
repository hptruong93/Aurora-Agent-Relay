from scapy.all import *
from abc import *

class Sniffer:
    __metaclass__ = ABCMeta

    def __init__(self, in_interface, out_interface):
        print "Init sniffer from %s outputting to %s" % (str(in_interface), str(out_interface))
        self.in_interface = in_interface
        self.out_interface = out_interface
        self.socket = conf.L2socket(iface = out_interface)

    def sniffing(self):
        sniff(iface=self.in_interface, store = 0, prn=lambda x: self.process(x))

    @abstractmethod
    def process(self, pkt):
        pass

if __name__ == '__main__':
    sniffer = Sniffer()
    sniffer.sniffing()
