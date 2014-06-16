from scapy.all import *
import sys
import traceback
import association_generator
import authentication_generator
import probe_request_generator

class WARPControlHeader(Packet):
    name = "WARPControlHeader"
    fields_desc = [ XByteField("power", 0),
                   XByteField("rate", 1),
                   XByteField("channel", 1),
                   XByteField("flag", 2)
                 ]

#####################################################################################################

WARP = "a0:a1:a2:a3:a4:a5"
PCEngine = "c0:c1:c2:c3:c4:c5"
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
def get_default_header(src = DEFAULT_SRC, dst = DEFAULT_DST):
    output = Ether() / WARPControlHeader()
    output.src = src
    output.dst = dst
    return output

#####################################################################################################

#See more on https://github.com/d1b/scapy/blob/master/scapy/layers/dot11.py
def get_association_request(src = WARP, dst = PCEngine, ssid_input = 'test'):
    return get_default_header(src, dst) / association_generator.generate(src, dst, ssid = ssid_input)

def get_reassociation_request(src = WARP, dst = PCEngine):
    return None

def get_probe_request(src = WARP, dst = PCEngine):
    return get_default_header(src, dst) / probe_request_generator.generate(src, dst)

def get_disassociation(src = WARP, dst = PCEngine):
    return None

def get_authentication(src = WARP, dst = PCEngine):
    return get_default_header(src, dst) / authentication_generator.generate(src, dst)

#####################################################################################################
class ToHostapd:#Get message from ethernet and put it into wlan0

    def __init__(self, in_interface = "eth4", out_interface = "mon.wlan0"):
        print "init sniffer"
        self.in_interface = in_interface
        self.out_interface = out_interface

    def sniffing(self):
        sniff(iface=self.in_interface, prn=lambda x: self._process(x))

    def _process(self, pkt):
        #print pkt.dst
        if True:
            tempWARP = WARPControlHeader(str(pkt.payload))
            dot11_frame = RadioTap(str(tempWARP.payload))
            sendp(dot11_frame, iface=self.out_interface)

#####################################################################################################
class ToWARP:#Get message from hwsim0 and output it to ethernet
    FILTER = VWIFI

    def __init__(self, in_interface = hwsim0, out_interface = "eth4", src = WIFISRC, dst = WARP):
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
class WARPDecodeFromPC:
    def __init__(self, in_interface = "eth4", out_interface = hwsim0):
        print "init WARPDecode"
        self.in_interface = in_interface
        self.out_interface = out_interface

    def sniffing(self):
        sniff(iface=self.in_interface, prn=lambda x: self._process(x))

    def _process(self, pkt):
        if type(pkt) == Dot3:
            Wpacket = WARPControlHeader(str(pkt.payload))
            outPacket = Dot11(str(Wpacket.payload))
            sendp(outPacket, iface=self.out_interface)
        else:
            print('Error: Unexpected type...\nSkipping Packet...')
#####################################################################################################
def strip_name(getFunction):
    return getFunction.__name__[3:]


def print_usage():
    print "Usage:"
    print "\tf/from_PC --> start processing packets originated from PC"
    print "\tt/to_PC --> start processing packets coming to PC from WARP"
    print "\tw/WARP_mode --> strip packet headers and forward dot11 management packet to monitor interface"
    print "\ts/send to_send iface [src] [dst] -l/--loop [number] --> send a packet to the desired interface" 
    print "\t\twhere to_send is one of the following: asc, reasc, probe, disasc, auth"
    print "\t\tiface is the interface"
    print "\t\tsrc and dst are optional, specifying what src and dst the packet will take. But they must go together (cannot specify just one)"
    print "\t\tloop number is also optional. If none provided then assume infinite loop"

if __name__ == '__main__':
    sending = {}
    sending['asc'] = get_association_request
    sending['reasc'] = get_reassociation_request
    sending['probe'] = get_probe_request
    sending['disasc'] = get_disassociation
    sending['auth'] = get_authentication
    
    if sys.argv[1] == "--help" or sys.argv[1] == "help" or sys.argv[1] == "-h":
        print_usage()
    elif sys.argv[1] == "f" or sys.argv[1] == "from_PC": #Start processing pkt originated from PC
        sniffer = ToWARP()
        sniffer.sniffing()
    elif sys.argv[1] == "t" or sys.argv[1] == "to_PC": #Start processing pkt sent to PC from Warp
        sender = ToHostapd()
        sender.sniffing()
    elif sys.argv[1] == "w" or sys.argv[1] == "WARP_mode":
        Wdecode = WARPDecodeFromPC()
        Wdecode.sniffing()
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
                    print "Send packet of type %s from src %s and dst %s with count = %s" % (strip_name(message_factory), src, dst, sys.argv[7])
                    sendp(message_factory(src, dst), iface=iface, count = int(sys.argv[7]))
                else:
                    print "Send packet of type %s from src %s and dst %s with infinite loop" % (strip_name(message_factory), src, dst)
                    sendp(message_factory(src, dst), iface=iface, loop = 1)
            else:
                print "Send packet of type %s from src %s and dst %s with no loop" % (strip_name(message_factory), src, dst)
                sendp(message_factory(src, dst), iface=iface)
        else:
            if len(sys.argv) >= 5:
                if sys.argv[4] == "-l" or sys.argv[4] == "--loop":
                    if len(sys.argv) == 6:
                        print "Send packet of type %s with count = %s" % (strip_name(message_factory), sys.argv[5])
                        sendp(message_factory(), iface=iface, count = int(sys.argv[5]), inter=0.1)
                    else:
                        print "Send packet of type %s with infinite loop" % (strip_name(message_factory))
                        sendp(message_factory(), iface=iface, loop = 1)
            else:
                print "Send packet of type %s with no loop" % (strip_name(message_factory))
                sendp(message_factory(), iface=iface)
