from scapy.all import *
import config

import WARPProtocol as WarpHeader
import association_generator
import authentication_generator
import probe_request_generator

#####################################################################################################
def get_default_header(src = config.WARP, dst = config.PCEngine):
    output = Ether() / WarpHeader.Transmit()
    output.src = src
    output.dst = dst
    return output

#####################################################################################################
#See more on https://github.com/d1b/scapy/blob/master/scapy/layers/dot11.py
def get_association_request(src = config.WARP, dst = config.PCEngine, ssid_input = config.CONFIG['hostapd']['ssid']):
    return get_default_header(src, dst) / association_generator.generate(ssid = ssid_input)

def get_reassociation_request(src = config.WARP, dst = config.PCEngine):
    return None

def get_probe_request(src = config.WARP, dst = config.PCEngine):
    return get_default_header(src, dst) / probe_request_generator.generate()

def get_disassociation(src = config.WARP, dst = config.PCEngine):
    return None

def get_authentication(src = config.WARP, dst = config.PCEngine):
    return get_default_header(src, dst) / authentication_generator.generate()
