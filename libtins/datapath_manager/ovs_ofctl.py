# OpenVSwitchController module
# Author: Hoai Phuoc Truong

# If you wish to add functionaility for custom commands
# - say abstracting a long multi-parameter command to a simple one-argument function
# i.e. application do_something --with=1234 --output=test.db "file.test" -X
# to module.do_this(1234,test.db)
# Simply modify the modify_bridge and modify_port commands to properly parse
# and format the command line.  The current parsing is done as simple if statements,
# but more complicated cases (i.e. optional parameters) can easily be added.

import subprocess
import os
import re

class OpenVSwitchController:
    """OpenVSwitchController module.  Provide programming interface for ovs-ofctl, allowing
    for the creation and modification of flows and ports mangement.
    Designed for OVS v 1.9.0"""
    
    MOD_PORT_ACTIONS = ['up', 'down', 'stp', 'nostp', 'receive', 'noreceive', 'noreceivestp', 
                        'forward','noforward', 'flood', 'noflood', 'packetin', 'nopacketin']

    """ Initialize a programming API for ovs-ofctl
        ovs: string representing the ovs name
    """
    def __init__(self, ovs, start = False):
        self.ovs_bridge = ovs
        if start:
            self.start()
    
    def _exec_command(self, args):
        # Want to throw exception if we give an invalid command
        # Note: args is a list of arguments
        # Format: [ arg1, arg2, arg3...]
        self._exec_command_with_output(args) #Don't care output
    
    def _exec_command_with_output(self, args):
        command = ["ovs-ofctl"]
        command.extend(args)
        print "\n  $ "," ".join(command)
        try:
            output = subprocess.check_output(command)
            return output
        except:
            print "Invalid call..."
            return None

    def start(self):
        """Start by removing all flows in the ovs."""
        self._exec_command(['del-flows', self.ovs_bridge])

    def stop(self):
        "Nothing to do here. Ovs should be able to shutdown by itself"
        pass

    def _flow_to_string(self, flow):
        flow_in_string = ""
        actions = None
        for key in flow:
            if key != "actions":
                flow_in_string += "%s=%s," % (key,flow[key])
            else:
                actions = flow[key]

        #Since actions has to always be at the end
        if actions is not None:
            flow_in_string += "%s=%s," % ("actions", actions)

        return flow_in_string[:-1]

    def get_flows(self):
        flow_string = self._exec_command_with_output(['dump-flows', self.ovs_bridge])
        return flow_string.split('\n')[1:]

    def add_flow(self, flow):
        #Construct flow from dict
        flow_string = self._flow_to_string(flow)
        self._exec_command(['add-flow', self.ovs_bridge, flow_string])

    def remove_flow(self, flow = None):
        if flow is not None:
            #Construct flow from dict
            flow_string = self._flow_to_string(flow)
            self._exec_command(['del-flows', self.ovs_bridge, flow_string])
        else:
            self._exec_command(['del-flows', self.ovs_bridge])

    def get_ports(self):
        """Get the list of all ports and their names. Return in a list ordered in the same way that ovs-ofctl show orders."""
        port_list = self._exec_command_with_output(["show", self.ovs_bridge])
        print port_list
        filtered_list = re.findall('[0-9][0-9]*\(.*\):', port_list)
        return [(item.split('(')[1])[:-2] for item in filtered_list]

    def get_port(self, interface_name):
        """Return the index of the port in the get_ports list, -1 if interface is not bounded to ovs"""
        try:
            return self.get_ports().index(interface_name)
        except ValueError:
            return -1

    def mod_port(self, port, action):
        if action in self.MOD_PORT_ACTIONS:
            self._exec_command(['mod-port', self.ovs_bridge, port, action])
        else:
            print "Cannot mod port with invalid action %s" % action