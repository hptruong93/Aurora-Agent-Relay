from scapy.all import *

def pr(pkt):
    if pkt.type == 0x8ae and pkt.dst == '00:0a:cd:21:0b:64':
        pl = str(pkt.payload)[4:]
        dot11 = Dot11(pl)
        if dot11.subtype != 8:
            print "-------------Type ------------------------> %s %s" % (dot11.type, dot11.subtype)
            print "-------------Length ----------------------> %s" % len(pkt)
        #if dot11.type != 0:
            #dot11.show()


sniff(iface = 'eth1', prn = pr, store = 0)

