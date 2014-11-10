# datapath_manager.py module
# See print_help() method below for usage
# Author: Hoai Phuoc Truong

import sys
import ovs, ovs_ofctl
import subprocess
import traceback

SUPPORTED_PROTOCOL = [0x0800, 0x0806, 0x86dd] #IP, ARP, IPv6

def execute(command):
    print "$ %s" % " ".join(command)
    subprocess.call(command)

def init(args):
    switch = ovs.OpenVSwitch()

    print "Creating bridge"
    switch.create_bridge(args[0])

    ovs_controller = ovs_ofctl.OpenVSwitchController(args[0], start = True)

    a = raw_input("Enter 'exit' to terminate")
    switch.stop()

def _add_interface(socket_path, bridge, interface_name):
    command = ["ovs-vsctl", "--db=unix:" + socket_path, "add-port", bridge, interface_name]
    execute(command)

def _remove_interface(socket_path, bridge, interface_name):
    command = ["ovs-vsctl", "--db=unix:" + socket_path, "del-port", bridge, interface_name]
    execute(command)

def show(args):
    #$ python datapath_manager.py show socket_path
    command = ["ovs-vsctl", "--db=unix:" + args[0], "show"]
    execute(command)

def show_flows(args):
    #$ python datapath_manager.py show-flows ovs_name
    ovs = ovs_ofctl.OpenVSwitchController(args[0])
    for flow in ovs.get_flows():
        print flow

def add(args):
    #"$ python datapath_manager.py add socket_path ovs_name virtual_interface ethernet_interface
    #Add the interface
    ovs_controller = ovs_ofctl.OpenVSwitchController(args[1])
    eth_interface_added = False

    ethernet_interface_index = ovs_controller.get_port(args[3]) + 1
    if ethernet_interface_index == 0: #ethernet interface
        _add_interface(args[0], args[1], args[3])
        eth_interface_added = True
        ethernet_interface_index = ovs_controller.get_port(args[3]) + 1

    virtual_interface_index = -1
    if ovs_controller.get_port(args[2]) == -1:
        _add_interface(args[0], args[1], args[2])
        virtual_interface_index = ovs_controller.get_port(args[2]) + 1
    else:
        print "Port already exists. Cannot add it again..."
        return

    #Turn off flooding
    if eth_interface_added:
        ovs_controller.mod_port(args[3], 'noflood')
    ovs_controller.mod_port(args[2], 'noflood')

    print "Add downlink flow for wlan %s to ethernet_interface %s" % (args[2], args[3])
    downlink = {}
    downlink['priority'] = '1'
    downlink['in_port'] = str(virtual_interface_index)
    downlink['actions'] = 'output:' + str(ethernet_interface_index)

    for protocol in SUPPORTED_PROTOCOL:
        downlink['dl_type'] = str(protocol)
        ovs_controller.add_flow(downlink)

def associate(args):
    #$ python datapath_manager.py associate socket_path ovs_name virtual_interface ethernet_interface mac_address
    ovs_controller = ovs_ofctl.OpenVSwitchController(args[1])
    virtual_interface_index = ovs_controller.get_port(args[2]) + 1
    ethernet_interface_index = ovs_controller.get_port(args[3]) + 1

    if virtual_interface_index == 0:
        print "Interface %s was not bounded to ovs %s" % (args[2], args[1])
    if ethernet_interface_index == 0:
        print "Interface %s was not bounded to ovs %s" % (args[3], args[1])

    print "Add uplink flow for mac address %s" % args[4]
    uplink = {}
    uplink['priority'] = '1'
    uplink['in_port'] = str(ethernet_interface_index)
    uplink['dl_src'] = str(args[4])
    uplink['actions'] = 'output:' + str(virtual_interface_index)

    for protocol in SUPPORTED_PROTOCOL:
        uplink['dl_type'] = str(protocol)
        ovs_controller.add_flow(uplink)


def check(args):
    #$ python datapath_manager.py check ovs_name interface_name
    ovs_controller = ovs_ofctl.OpenVSwitchController(args[0])
    #Check if port exists
    if ovs_controller.get_port(args[1]) == -1:
        print -1
    else:
        print 0

def disassociate(args):
    #$ python datapath_manager.py disassociate socket_path ovs_name virtual_interface ethernet_interface mac_address
    ovs_controller = ovs_ofctl.OpenVSwitchController(args[1])
    virtual_interface_index = ovs_controller.get_port(args[2]) + 1
    ethernet_interface_index = ovs_controller.get_port(args[3]) + 1

    if virtual_interface_index == 0:
        print "Interface %s was not bounded to ovs %s" % (args[2], args[1])
    if ethernet_interface_index == 0:
        print "Interface %s was not bounded to ovs %s" % (args[3], args[1])

    print "Remove uplink for mac address %s" % args[4]
    uplink = {}
    uplink['in_port'] = str(ethernet_interface_index)
    uplink['dl_src'] = str(args[4])

    for protocol in SUPPORTED_PROTOCOL:
        uplink['dl_type'] = str(protocol)
        ovs_controller.remove_flow(uplink)


def remove(args):
    #$ python datapath_manager.py remove socket_path ovs_name virtual_interface ethernet_interface
    ovs_controller = ovs_ofctl.OpenVSwitchController(args[1])
    virtual_interface_index = ovs_controller.get_port(args[2]) + 1
    ethernet_interface_index = ovs_controller.get_port(args[3]) + 1

    if virtual_interface_index == 0:
        print "Interface %s was not bounded to ovs %s" % (args[2], args[1])
    if ethernet_interface_index == 0:
        print "Interface %s was not bounded to ovs %s" % (args[3], args[1])

    print "Remove downlink for interface %s" % args[3]
    downlink = {}
    downlink['in_port'] = str(virtual_interface_index)

    for protocol in SUPPORTED_PROTOCOL:
        downlink['dl_type'] = str(protocol)
        ovs_controller.remove_flow(downlink)

    print "Removing wlan interface %s" % args[2]
    _remove_interface(args[0], args[1], args[2])

def clear(args):
    #$ python datapath_manager.py clear ovs_name
    ovs_controller = ovs_ofctl.OpenVSwitchController(args[0])
    ovs_controller.remove_flow()

def print_help():
    print "-----------------------------datapath_manager module----------------------------------------"
    print "This manager uses ovs-vsctl and ovs-ofctl to construct a path between WARP and the hwsim wlan."
    print "This script should be called indepently. Therefore there is no class construction and encapsulation."
    print "Usages --> [as root]"
    print "$ python datapath_manager.py init ovs_name"
    print "$ python datapath_manager.py show socket_path"
    print "$ python datapath_manager.py show-flows ovs_name"
    print "$ python datapath_manager.py add socket_path ovs_name virtual_interface ethernet_interface"
    print "$ python datapath_manager.py associate socket_path ovs_name virtual_interface ethernet_interface mac_address"
    print "$ python datapath_manager.py check ovs_name interface_name"
    print "$ python datapath_manager.py disassociate socket_path ovs_name virtual_interface ethernet_interface mac_address"
    print "########################## WARNING #########################################################"
    print "# Before calling remove command below, caller has to disassociate all previously associated mac address by"
    print "# calling the disassociate command above. Manager is not responsible for removing uplink flows left behind."
    print "$ python datapath_manager.py remove socket_path ovs_name virtual_interface ethernet_interface"
    print "############################################################################################"
    print "$ python datapath_manager.py clear ovs_name"
    print "See ovs.py for what socket_path means --> Path to the socket file created by the init operation previously"
    print "--------------------------------------------------------------------------------------------"


if __name__ == "__main__":
    function_mapping = {
        "init" : init,
        "show" : show,
        "show-flows" : show_flows,
        "add" : add,
        "associate" : associate,
        "check" : check,
        "disassociate" : disassociate,
        "remove" : remove,
        "clear" : clear
    }

    try:
        if sys.argv[1] == "--help" or sys.argv[1] == "-h":
            print_help()
        else:
            calling = function_mapping.get(sys.argv[1])
            calling(sys.argv[2:])        
    except:
        print_help()
        print "Encountered exception..."
        traceback.print_exc(file=sys.stdout)
