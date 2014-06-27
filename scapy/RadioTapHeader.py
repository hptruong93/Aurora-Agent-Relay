from scapy.all import *

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
    radioTap.len = 18
    radioTap.present = ((1 << FLAGS) | (1 << RATE) | (1 << CHANNEL))
    radioTap.notdecoded = '\x00\x02q\t\xa0\x00\xd7\x02\x00\x00' #But what do they mean??
    return radioTap