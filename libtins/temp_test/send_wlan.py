from scapy.all import *

def process(pkt):
	sendp(pkt, iface = 'eth1')

sniff(iface = 'wlan0', prn = process, store = 0)