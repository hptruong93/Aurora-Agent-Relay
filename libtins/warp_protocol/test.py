from scapy.all import *

def process(pkt):
	try:
		hexdump(pkt.payload)
		# pkt = str(pkt.payload)[5:]
		# pkt = IP(pkt)
		# print "--------------------------------------------------------------------------"
		# print "%s --> %s" % (pkt.src, pkt.dst)
		print "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++"
	except:
		pass

sniff(iface = 'eth0', prn = process, store = 0)