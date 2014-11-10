from scapy.all import *

bssid = '40:d8:55:04:22:84'
eth_src = '00:0d:b9:34:17:29'
eth_dst = bssid


def process(pkt):
	if pkt.addr2 == bssid:
		to_send = Ether()
		to_send.src = eth_src
		to_send.dst = eth_dst

		to_send = to_send / pkt.payload #Remove radio tap
		to_send.type = 0x8de

		sendp(to_send, iface = 'eth1')

sniff(iface = 'hwsim0', prn = process, store = 0)
