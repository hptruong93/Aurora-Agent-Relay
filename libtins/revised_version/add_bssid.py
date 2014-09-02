from scapy.all import *

a = Ether()
a.src = '00:0D:B9:34:17:29'
a.dst = '40:d8:55:04:22:85'
a.type = 0x8ae

a = a / Raw('\x02\x02\x01\x40\xd8\x55\x04\x22\x80')
sendp(a, iface = 'eth1')
