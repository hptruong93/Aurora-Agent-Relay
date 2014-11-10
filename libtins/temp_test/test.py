import subprocess

command = ['ovs-ofctl', 'show', 'tb']
a = subprocess.check_output(command)
print "A is "
print a
