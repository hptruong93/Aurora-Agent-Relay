import ovs
import subprocess

def execute(command):
    print "$%s" % " ".join(command)
    subprocess.call(command)

test = ovs.OpenVSwitch()

# command = 'vethd -e wlan0 -v w0'
# print '$' + command
# subprocess.check_call(command.split(' '))

# command = 'vethd -e eth1 -v w1'
# print '$' + command
# subprocess.check_call(command.split(' '))

print "Creating bridge"
test.create_bridge("tb")

print "Adding w0 as port 1"
# test.add_port("tb", "w0")

print "Adding w1 as port 2"
# test.add_port("tb", "w1")

print "Adding w2 as port 3"
# test.add_port("tb", "w2")


print "Delete flows"
# execute(['ovs-ofctl', 'del-flows', 'tb'])

phone_mac='00:AA:70:AE:A3:09'
laptop_mac='00:21:5D:22:97:8C'
slice0_dev = laptop_mac
slice1_dev = phone_mac

# print "Turn flooding off"
# execute(['ovs-ofctl', 'mod-port', 'tb', '1', 'noflood'])
# execute(['ovs-ofctl', 'mod-port', 'tb', '2', 'noflood'])
# execute(['ovs-ofctl', 'mod-port', 'tb', '3', 'noflood'])
# execute(['ovs-ofctl', 'mod-port', 'tb', '4', 'noflood'])

# print "Add up link flow for slice 0"
# execute(['ovs-ofctl', 'add-flow', 'tb', 'priority=1,in_port=2,dl_type=0x0800,dl_src=%s,actions=output:1' % slice0_dev])
# execute(['ovs-ofctl', 'add-flow', 'tb', 'priority=1,in_port=2,dl_type=0x0806,dl_src=%s,actions=output:1' % slice0_dev])
# execute(['ovs-ofctl', 'add-flow', 'tb', 'priority=1,in_port=2,dl_type=0x86dd,dl_src=%s,actions=output:1' % slice0_dev])

# print "Add up link flow for slice 1"
# execute(['ovs-ofctl', 'add-flow', 'tb', 'priority=1,in_port=2,dl_type=0x0800,dl_src=%s,actions=output:3' % slice1_dev])
# execute(['ovs-ofctl', 'add-flow', 'tb', 'priority=1,in_port=2,dl_type=0x0806,dl_src=%s,actions=output:3' % slice1_dev])
# execute(['ovs-ofctl', 'add-flow', 'tb', 'priority=1,in_port=2,dl_type=0x86dd,dl_src=%s,actions=output:3' % slice1_dev])

# print "Add down link flow for slices"
# execute(['ovs-ofctl', 'add-flow', 'tb', 'priority=1,in_port=1,dl_type=0x0800,actions=output:2'])
# execute(['ovs-ofctl', 'add-flow', 'tb', 'priority=1,in_port=1,dl_type=0x0806,actions=output:2'])
# execute(['ovs-ofctl', 'add-flow', 'tb', 'priority=1,in_port=1,dl_type=0x86dd,actions=output:2'])

# execute(['ovs-ofctl', 'add-flow', 'tb', 'priority=1,in_port=3,dl_type=0x0800,actions=output:2'])
# execute(['ovs-ofctl', 'add-flow', 'tb', 'priority=1,in_port=3,dl_type=0x0806,actions=output:2'])
# execute(['ovs-ofctl', 'add-flow', 'tb', 'priority=1,in_port=3,dl_type=0x86dd,actions=output:2'])

a = raw_input('Just waiting')
test.stop()