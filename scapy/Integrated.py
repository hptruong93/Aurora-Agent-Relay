#!/usr/bin/python

from scapy.all import *
import sys
import traceback

import WARPProtocol as WarpHeader
import RadioTapHeader as radio_tap

import Sniffer
import packet_factory as factory
import packet_util
import config

#####################################################################################################
DEFAULT_VWFACE = "wlan0"
DEAFULT_IFACE = "eth3"
eth0 = "eth0"
eth1 = "eth1"
eth2 = "eth2"
eth3 = "eth3"
eth4 = "eth4"
hwsim0 = "hwsim0"

#####################################################################################################
class ToHostapd(Sniffer.Sniffer):#Get message from ethernet and put it into wlan0

    def __init__(self, in_interface = config.CONFIG['to_hostapd']['in'], out_interface = config.CONFIG['to_hostapd']['out']):
        print "init tohostapd"
        super(ToHostapd, self).__init__(str(in_interface), str(out_interface))

    def process(self, pkt):
        to_me = False
        try:
            #Test if this packet comes from the WARP, directed toward the PC ethernet
            to_me = (pkt.dst == config.CONFIG['PC_mac']['ETH']) or (pkt.dst == config.CONFIG['general_mac']['BROADCAST'])
            #if to_me:
            #    print str(pkt.src) + " --> " + str(pkt.dst)
        except Exception as e:
            traceback.print_exc()
            to_me = False

        if to_me:
            tempWARP = WarpHeader.WARPHeader(str(pkt.payload))
            if int(tempWARP.type) != int(WarpHeader.TYPE_DEF['transmit']):
                return
            tempWARP = WarpHeader.Transmit(str(tempWARP.payload))

            try:
                inner = Dot11(str(tempWARP.payload))
                inner = packet_util.get_packet_header(tempWARP) / inner
                #inner.show()
                #hexdump(inner)
                pass
            except:
                tempWARP.show() 
                pass
            
            dot11_frame = radio_tap.get_default_radio_tap() / tempWARP.payload
            sendp(dot11_frame, iface=self.out_interface)
            #self.socket.send(dot11_frame)
            try:
                pass
                #if tempWARP.payload.type != 0:
                #    sendp(dot11_frame, iface = 'wlan1')
            except:
                pass

#####################################################################################################
class ToWARP(Sniffer.Sniffer):#Get message from hwsim0 and output it to ethernet

    def __init__(self, in_interface = config.CONFIG['to_warp']['in'], out_interface = config.CONFIG['to_warp']['out'], src = config.WIFISRC, dst = config.WARP):
        print "init towarp"
        self.src = str(src)
        self.dst = str(dst)
        
        self.warp_header = WarpHeader.WARPHeader()
        self.warp_header.type = 'transmit'
        
        self.count = 0
        self.last = packet_util.current_milli()
        
        super(ToWARP, self).__init__(str(in_interface), str(out_interface))

    def process(self, pkt):
        try:
            inner = Dot11(str(pkt.payload))

            #if inner.subtype == 5 and str(inner.addr1)[-2:] == "8c" and in_interface != 'hwsim0':
                #current_time = packet_util.current_milli()
                #self.count += 1
                #print "Average = %s" % (1000 * float (self.count) / (current_time - self.last))

            #inner.show()
            #hexdump(inner)
            #Check if messaged is from hostapd
            if inner.addr2 != config.CONFIG['WARP_mac']['eth_a']:
                #inner.show()
                return
        except:
            return
        
        
        warp_header = self.warp_header / WarpHeader.Transmit()
        if inner.addr1 == config.CONFIG['general_mac']['BROADCAST']:
            warp_header.payload.retry = config.CONFIG['transmission']['retry_broadcast']
        elif inner.type == 0:
            warp_header.payload.retry = config.CONFIG['transmission']['retry_management']
        else:
            warp_header.payload.retry = config.CONFIG['transmission']['retry_data']
	
        #inner.show()
        eth_frame = Ether() / warp_header / inner
        eth_frame.src = self.src
        eth_frame.dst = self.dst
        #sendp(eth_frame, iface = self.out_interface, verbose = 0)
        self.socket.send(eth_frame)
        
    def sniffing(self):
        if in_interface == 'hwsim0':
            sniff(iface=self.in_interface, store = 0, prn=lambda x: self.process(x), filter = 'type 0 subtype 8')
        else:
            sniff(iface=self.in_interface, store = 0, prn=lambda x: self.process(x), filter = 'type 0')

#####################################################################################################
class WARPDecodeFromPC(Sniffer.Sniffer):
    def __init__(self, in_interface = config.CONFIG['warp_decode']['in'], out_interface = config.CONFIG['warp_decode']['out']):
        print "init WARPDecode"
        super(WARPDecodeFromPC, self).__init__(str(in_interface), str(out_interface))

    def process(self, pkt):
        if type(pkt) == Dot3:
            Wpacket = WarpHeader.Transmit(str(pkt.payload))
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
    sending['asc'] = factory.get_association_request
    sending['reasc'] = factory.get_reassociation_request
    sending['probe'] = factory.get_probe_request
    sending['disasc'] = factory.get_disassociation
    sending['auth'] = factory.get_authentication
    
    if sys.argv[1] == "--help" or sys.argv[1] == "help" or sys.argv[1] == "-h":
        print_usage()
    elif sys.argv[1] == "f" or sys.argv[1] == "from" or sys.argv[1] == "from_PC": #Start processing pkt originated from PC
        sniffer = None
        if len(sys.argv) == 4:
            in_interface = sys.argv[2]
            out_interface = sys.argv[3]
            sniffer = ToWARP(in_interface = in_interface, out_interface = out_interface)
        else:
            sniffer = ToWARP()
        sniffer.sniffing()
    elif sys.argv[1] == "t" or sys.argv[1] == "to" or sys.argv[1] == "to_PC": #Start processing pkt sent to PC from Warp
        sender = None
        if len(sys.argv) == 4:
            in_interface = sys.argv[2]
            out_interface = sys.argv[3]
            sender = ToHostapd(in_interface = in_interface, out_interface = out_interface)
        else:
            sender = ToHostapd()
        sender.sniffing()
    elif sys.argv[1] == "w" or sys.argv[1] == "warp" or sys.argv[1] == "WARP_mode":
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
