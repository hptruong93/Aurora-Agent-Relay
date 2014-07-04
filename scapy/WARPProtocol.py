from scapy.all import *

class WARPControlHeader(Packet):
    name = "WARPControlHeader"
    fields_desc = [ XByteField("power", 0),
                   XByteField("rate", 1),
                   XByteField("channel", 1),
                   XByteField("flag", 2),
                   XByteField("retry", 0)
                 ]
