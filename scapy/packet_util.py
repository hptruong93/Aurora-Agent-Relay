from scapy.all import *

def get_packet_header(packet):
    return packet.__class__(_get_packet_header_in_string(packet))

def _get_packet_header_in_string(packet):
    l = len(packet) - len(packet.payload)
    return str(packet)[0:l]

def get_packet_layers(packet):
    output = []
    while (packet.payload):
        output.append(get_packet_header(packet))
        packet = packet.payload
    output.append(packet)
    return output

def reconstruct_packet(layer_list):
    output = None
    for item in layer_list:
        if output is None:
            output = item
        else:
            output = output / item
    return output

def remove_component_number(packet, index_list, packet_layers = None):
    """
        Remove certain layers in the packet that have the index in index_list
    """
    if packet_layers is None:
        packet_layers = get_packet_layers(packet)
    
    for index in sorted(index_list, reverse=True):
        del packet_layers[index]

    return reconstruct_packet(packet_layers)

def remove_component(packet, test_function):
    """
        Remove certain layers in the packet that satisfies the test_function
        test_function will be called on every layer of the packet. Therefore
        it is recommended to have a try except block in test_function
    """
    packet_layers = get_packet_layers(packet)

    index_list = []
    for i in xrange(0, len(packet_layers)):
        if test_function(packet_layers[i]):
            index_list.append(i)

    print "Removing %s" % index_list
    return remove_component_number(packet, index_list, packet_layers)