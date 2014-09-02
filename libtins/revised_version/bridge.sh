#!/bin/bash
#Usage: bash bridge.sh [-c] iface1 viface1 iface2 viface2 bridge_name
#This will bridge two interfaces and created a simple aurora slice for testing purpose

if [ "$1" == "-c" ] || [ "$1" == "--clean" ]; then
	echo Clean up interfaces...
	ifconfig ${viface1} down
	ifconfig ${viface2} down
	killall vethd
	shift
fi

iface1=${1}
shift
viface1=${1}
shift
iface2=${1}
shift
viface2=${1}
shift
bridge_name=${1}
shift




echo Setting up interface ${iface1} with virtual interface ${viface1}
echo vethd -v ${viface1} -e ${iface1}
vethd -v ${viface1} -e ${iface1}
ifconfig ${viface1} up

echo Setting up interface ${iface2} with virtual interface ${viface2}
echo vethd -v ${viface2} -e ${iface2}
vethd -v ${viface2} -e ${iface2}
ifconfig ${viface2} up


echo Clean up bridge named ${bridge_name}
ifconfig ${bridge_name} down
brctl delbr ${bridge_name}

echo Creating new bridge
brctl addbr ${bridge_name}
brctl addif ${bridge_name} ${viface1}
brctl addif ${bridge_name} ${viface2}
ifconfig ${bridge_name} up
