import json
import os
CONFIG = json.load(open(os.path.dirname(os.path.abspath(__file__))+ '/.config'))

WARP = CONFIG['WARP_mac']['virtual_mac']
PCEngine = CONFIG['PC_mac']['virtual_mac']
VWIFI = CONFIG['WARP_mac']['eth_a'] #Virtual Wifi Interface

WIFIDST = VWIFI
WIFISRC = CONFIG['general_mac']['WIFI_SRC'] #Random for now
