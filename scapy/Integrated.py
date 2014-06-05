from scapy.all import *
import sys
import traceback

class WARPControlHeader(Packet):
    name = "WARPControlHeader"
    fields_desc = [ XByteField("power", 0),
                   XByteField("rate", 1),
                   XByteField("channel", 1),
                   XByteField("flag", 2)
                 ]

#####################################################################################################

WARP = "a0:a1:a2:a3:a4:a5"
PCEngine = "0a:00:27:00:00:00"
VWIFI = "02:00:00:00:00:00" #Virtual Wifi Interface

DEFAULT_SRC = WARP
DEFAULT_DST = PCEngine
WIFIDST = VWIFI
WIFISRC = "c0:c1:c2:c3:c4:c5" #Random for now

BROADCAST = "ff:ff:ff:ff:ff:ff"
DEFAULT_VWFACE = "wlan0"
DEAFULT_IFACE = "eth3"
RADIO_TAP_HEADER = "\x00\x00\x0e\x00\x0e\x00\x00\x00\x00\x02\x80\t\xa0\x00"

eth0 = "eth0"
eth1 = "eth1"
eth2 = "eth2"
eth3 = "eth3"
hwsim0 = "hwsim0"

#####################################################################################################

def getDefaultHeader(src = DEFAULT_SRC, dst = DEFAULT_DST):
    output = Ether() / WARPControlHeader()
    output.src = src
    output.dst = dst
    return output

def addWifiInfo(packet, src = WIFISRC, dst = WIFIDST):
    packet.src = src
    packet.dst = dst
    return packet

#####################################################################################################

#See more on https://github.com/d1b/scapy/blob/master/scapy/layers/dot11.py
def getAssociationRequest(src = WARP, dst = PCEngine):
    return getDefaultHeader(src, dst) / addWifiInfo(Dot11AssoReq(), src, dst)

def getReAssociationRequest(src = WARP, dst = PCEngine):
    return getDefaultHeader(src, dst) / addWifiInfo(Dot11ReassoReq(), src, dst)

def getProbeRequest(src = WARP, dst = PCEngine):
    return getDefaultHeader(src, dst) / addWifiInfo(Dot11ProbeReq(), src, dst)

def getDisassociation(src = WARP, dst = PCEngine):
    return getDefaultHeader(src, dst) / addWifiInfo(Dot11Disas(), src, dst)

def getAuthentication(src = WARP, dst = PCEngine):
    return getDefaultHeader(src, dst) / addWifiInfo(Dot11Auth(), src, dst)

#EthernetII assocPkt = EthernetII(ETHDST, ETHSRC) / WARPControlPDU() /   Dot11AssocRequest(WIFIDST, WIFISRC);

#####################################################################################################
class Sender:#Get message from ethernet and put it into wlan0

    def __init__(self, in_interface = eth1, out_interface = DEFAULT_VWFACE):
        print "init sniffer"
        self.in_interface = in_interface
        self.out_interface = out_interface

    def sniffing(self):
        sniff(iface=self.in_interface, prn=lambda x: self._process(x))

    def _process(self, pkt):
        #print pkt.dst
        if pkt.dst == BROADCAST:
            dot11_frame = RadioTap(RADIO_TAP_HEADER) / Dot11(str(pkt.payload))
#            print dot11_frame.show()
            sendp(dot11_frame, iface=self.out_interface)

#####################################################################################################
class Sniffer:#Get message from hwsim0 and output it to ethernet
    FILTER = VWIFI

    def __init__(self, in_interface = hwsim0, out_interface = eth1, src = WIFISRC, dst = WARP):
        print "init sniffer"
        self.in_interface = in_interface
        self.out_interface = out_interface
        self.src = src
        self.dst = dst

    def sniffing(self):
        sniff(iface=self.in_interface, prn=lambda x: self._process(x))

    def _process(self, pkt):
        if pkt.payload.addr2 == self.FILTER or pkt.payload.addr2 == BROADCAST:
            eth_frame = Ether() / WARPControlHeader() /str(pkt.payload)
            eth_frame.src = self.src
            eth_frame.dst = self.dst
            sendp(eth_frame, iface = self.out_interface)

#####################################################################################################

def print_usage():
    print "Usage:"
    print "f/from_PC --> start processing packets originated from PC"
    print "t/to_PC --> start processing packets coming to PC from WARP"
    print "s to_send iface [src] [dst] -l/--loop [number] --> send a packet to the desired interface" 
    print "where to_send is one of the following: asc, reasc, probe, disasc, auth"
    print "iface is the interface"
    print "src and dst are optional, specifying what src and dst the packet will take. But they must go together (cannot specify just one)"
    print "loop number is also optional. If none provided then assume infinite loop"

if __name__ == '__main__':
    sending = {}
    sending['asc'] = getAssociationRequest
    sending['reasc'] = getReAssociationRequest
    sending['probe'] = getProbeRequest
    sending['disasc'] = getDisassociation
    sending['auth'] = getAuthentication
    
    if sys.argv[1] == "--help" or sys.argv[1] == "help" or sys.argv[1] == "-h":
        print_usage()
    elif sys.argv[1] == "f" or sys.argv[1] == "from_PC": #Start processing pkt originated from PC
        sniffer = Sniffer()
        sniffer.sniffing()
    elif sys.argv[1] == "t" or sys.argv[1] == "to_PC": #Start processing pkt sent to PC from Warp
        sender = Sender()
        sender.sniffing()
    elif sys.argv[1] == "s" or sys.argv[1] == "send":
        message_factory = None
        iface = None
        try:
            message_factory = sending[sys.argv[2]]
            iface = sys.argv[3]
        except Exception as e:
            print "Missing something"
            traceback.print_exc(file=sys.stdout)
            print_usage()
            sys.exit(-1)

        src = None
        dst = None
        try:
            src = sys.argv[4]
            if src == "-l" or src == "--loop":
                src = None
            dst = sys.argv[5]
        except Exception as e:
            pass

        print "Sending through interface %s" % iface
        if src and dst:
            if sys.argv[6] == "-l" or sys.argv[6] == "--loop":
                if len(sys.argv) == 8:
                    print "Send packet of type %s from src %s and dst %s with count = %s" % (message_factory, src, dst, sys.argv[7])
                    sendp(message_factory(src, dst), iface=iface, count = int(sys.argv[7]))
                else:
                    print "Send packet of type %s from src %s and dst %s with infinite loop" % (message_factory, src, dst)
                    sendp(message_factory(src, dst), iface=iface, loop = 1)
            else:
                print "Send packet of type %s from src %s and dst %s with no loop" % (message_factory, src, dst)
                sendp(message_factory(src, dst), iface=iface)
        else:
            if len(sys.argv) >= 5:
                if sys.argv[4] == "-l" or sys.argv[4] == "--loop":
                    if len(sys.argv) == 6:
                        print "Send packet of type %s with count = %s" % (message_factory, sys.argv[5])
                        sendp(message_factory(), iface=iface, count = int(sys.argv[5]))
                    else:
                        print "Send packet of type %s with infinite loop" % (message_factory)
                        sendp(message_factory(), iface=iface, loop = 1)
            else:
                print "Send packet of type %s with no loop" % (message_factory)
                sendp(message_factory(), iface=iface)
