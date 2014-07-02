#!/usr/bin/python

from scapy.all import *
import sys
import traceback

import WARPProtocol as WarpHeader
import association_generator
import authentication_generator
import probe_request_generator
import RadioTapHeader as radio_tap

import packet_util
import config

#####################################################################################################

WARP = config.CONFIG['WARP_mac']['virtual_mac']
PCEngine = config.CONFIG['general_mac']['PC']
VWIFI = config.CONFIG['WARP_mac']['eth_a'] #Virtual Wifi Interface

DEFAULT_SRC = WARP
DEFAULT_DST = PCEngine
WIFIDST = VWIFI
WIFISRC = config.CONFIG['general_mac']['WIFI_SRC'] #Random for now

BROADCAST = "ff:ff:ff:ff:ff:ff"
DEFAULT_VWFACE = "wlan0"
DEAFULT_IFACE = "eth3"
RADIO_TAP_HEADER = "\x00\x00\x0e\x00\x0e\x00\x00\x00\x00\x02\x80\t\xa0\x00"

eth0 = "eth0"
eth1 = "eth1"
eth2 = "eth2"
eth3 = "eth3"
eth4 = "eth4"
hwsim0 = "hwsim0"

#####################################################################################################
def get_default_header(src = DEFAULT_SRC, dst = DEFAULT_DST):
    output = Ether() / WarpHeader.WARPControlHeader()
    output.src = src
    output.dst = dst
    return output

#####################################################################################################

#See more on https://github.com/d1b/scapy/blob/master/scapy/layers/dot11.py
def get_association_request(src = WARP, dst = PCEngine, ssid_input = config.CONFIG['hostapd']['ssid']):
    return get_default_header(src, dst) / association_generator.generate(ssid = ssid_input)

def get_reassociation_request(src = WARP, dst = PCEngine):
    return None

def get_probe_request(src = WARP, dst = PCEngine):
    return get_default_header(src, dst) / probe_request_generator.generate()

def get_disassociation(src = WARP, dst = PCEngine):
    return None

def get_authentication(src = WARP, dst = PCEngine):
    return get_default_header(src, dst) / authentication_generator.generate()

#####################################################################################################
class ToHostapd:#Get message from ethernet and put it into wlan0

    def __init__(self, in_interface = config.CONFIG['to_hostapd']['in'], out_interface = config.CONFIG['to_hostapd']['out']):
        print "init to hostapd"
        self.in_interface = str(in_interface)
        self.out_interface = str(out_interface)

    def sniffing(self):
        sniff(iface=self.in_interface, store = 0, prn=lambda x: self._process(x))

    def _process(self, pkt):
        tempWARP = WarpHeader.WARPControlHeader(str(pkt.payload))
        passing = False
        try:
            #Test if this packet comes from the WARP, directed toward the PC ethernet
            passing = (pkt.dst == config.CONFIG['PC_mac']['ETH']) or (pkt.dst == config.CONFIG['general_mac']['BROADCAST'])
            if passing:
            #    print str(pkt.src) + " --> " + str(pkt.dst)
                pass
        except Exception as e:
            traceback.print_exc()
            passing = False
        if passing:
            #try:
                #x = Dot11(str(tempWARP.payload))
                #x = packet_util.get_packet_header(tempWARP) / x
                #x.show()
            #except:
                #tempWARP.show() 
            dot11_frame = radio_tap.get_default_radio_tap() / tempWARP.payload
            sendp(dot11_frame, iface=self.out_interface)

#####################################################################################################
class ToWARP:#Get message from hwsim0 and output it to ethernet
    FILTER = VWIFI

    def __init__(self, in_interface = config.CONFIG['to_warp']['in'], out_interface = config.CONFIG['to_warp']['out'], src = WIFISRC, dst = WARP):
        print "init towarp"
        self.in_interface = str(in_interface)
        self.out_interface = str(out_interface)
        self.src = str(src)
        self.dst = str(dst)

    def sniffing(self):
        sniff(iface=self.in_interface, store = 0, prn=lambda x: self._process(x))

    def _process(self, pkt):
        if True:
            try:
                inner = Dot11(str(pkt.payload))
                #inner.show()
                #Check if messaged is from hostapd
                if inner.addr2 != config.CONFIG['WARP_mac']['eth_a']:
                    #inner.show()
                    return
            except:
                return
            eth_frame = Ether() / WarpHeader.WARPControlHeader() / str(pkt.payload)
            eth_frame.src = self.src
            eth_frame.dst = self.dst
            sendp(eth_frame, iface = self.out_interface)

#####################################################################################################
class WARPDecodeFromPC:
    def __init__(self, in_interface = config.CONFIG['warp_decode']['in'], out_interface = config.CONFIG['warp_decode']['out']):
        print "init WARPDecode"
        self.in_interface = str(in_interface)
        self.out_interface = str(out_interface)

    def sniffing(self):
        sniff(iface=self.in_interface, store = 0, prn=lambda x: self._process(x))

    def _process(self, pkt):
        if type(pkt) == Dot3:
            Wpacket = WarpHeader.WARPControlHeader(str(pkt.payload))
            outPacket = Dot11(str(Wpacket.payload))
            sendp(outPacket, iface=self.out_interface)
        else:
            print('Error: Unexpected type...\nSkipping Packet...')
#####################################################################################################
def strip_name(getFunction):
    return getFunction.__name__[4:]


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
