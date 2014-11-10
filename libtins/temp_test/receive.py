from scapy.all import *

mon_type = 0x8ae#0x8cf
arp_type = 0x8af
ip_type = 0x8bf

real_arp = 0x0806
real_ip = 0x0800

bssid1 = '40:d8:55:04:22:84'
bssid2 = '40:d8:55:04:22:86'

eth_src = '00:0d:b9:34:17:29'
eth_dst = '40:d8:55:04:22:84'


FLAGS = 1
RATE = 2
CHANNEL = 3
ANTENNA_SIGNAL = 5
ANTENNA_NOISE = 6
ANTENNA = 11
DB_ANTENNA_SIGNAL = 12
DB_ANTENNA_NOISE = 13
RX_FLAG = 14

def get_default_radio_tap():
    radioTap = RadioTap()
    radioTap.len = 14
    radioTap.present = ((1 << FLAGS) | (1 << RATE) | (1 << CHANNEL))
    radioTap.notdecoded = '\x00\x02q\t\xa0\x00'
    return radioTap

default_tap = get_default_radio_tap()

def process(pkt):
    if pkt.type == mon_type and pkt.src != eth_src:
        to_send = default_tap / Dot11(str(pkt.payload)[4:])
        if to_send.payload.addr1 == bssid1:
            sendp(to_send, iface = 'mon.wlan0')
        elif to_send.payload.addr1 == bssid2:
            sendp(to_send, iface = 'mon.wlan1')
        elif to_send.payload.addr1 == 'ff:ff:ff:ff:ff:ff':
            sendp(to_send, iface = 'mon.wlan0')

sniff(iface = 'eth1', prn = process, store = 0)