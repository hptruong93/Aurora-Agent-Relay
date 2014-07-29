from scapy.all import *
import os
import json
config = json.load(open(os.path.dirname(os.path.abspath(__file__))+ '/.config_warp_protocol'))

def _reverse_dict(dictionary):
    return dict(reversed(item) for item in dictionary.items())
    
TYPE_DEF = _reverse_dict(config['header']['type']['definition'])
CONTROL_DEF = _reverse_dict(config['header']['subtype']['definition']['control'])
MAC_CONTROL_OPERATIONS = _reverse_dict(config['MAC_control']['operation']['definition'])


def _fix_json_load(loaded_json):
    output = {}
    for key in loaded_json:
        output[int(key)] = loaded_json[key]
    return output

class WARPHeader(Packet):
    name = "WARPHeader"
    fields_desc = [ ByteEnumField("type", config['header']['type']['default'], _fix_json_load(config['header']['type']['definition'])),
                   XByteField("subtype", config['header']['subtype']['default']) 
                 ]
class Transmit(Packet):
    name = "WARPControlHeader"
    fields_desc = [ XByteField("power", config['transmit']['power']['default']),
                   XByteField("rate", config['transmit']['rate']['default']),
                   XByteField("channel", config['transmit']['channel']['default']),
                   XByteField("flag", config['transmit']['flag']['default']),
                   XByteField("retry", config['transmit']['retry']['default']),
                   XByteField("length_msb", config['transmit']['length_msb']['default']),
                   XByteField("length_lsb", config['transmit']['length_lsb']['default'])
                 ]

class MACAddressControl(Packet):
    name = "MACAddrControl"
    fields_desc = [ ByteEnumField("operation", config['MAC_control']['operation']['default'], _fix_json_load(config['MAC_control']['operation']['definition'])),
                   MACField("mac_address", config['MAC_control']['mac_address']['default'])
                 ]


if __name__ == "__main__":
    pass
