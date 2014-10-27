#!/bin/bash
#Usage: bash bridge.sh [-c] iface1 viface1 iface2 viface2 bridge_name
#This will bridge two interfaces and created a simple aurora slice for testing purpose

if [ "$1" == "-c" ] || [ "$1" == "--clean" ]; then
        shift
	echo Clean up interfaces...

        while [[ -n "$1" ]]; do
                ifconfig ${1} down
                shift
        done

	killall vethd
	shift
elif [ "$1" == "-cb" ] || [ "$1" == "--clean-bridge" ]; then
        shift
        echo Clean up bridges...
        
        while [[ -n "$1" ]]; do
                ifconfig ${1} down
                brctl delbr ${1}
                shift
        done
else

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

echo Add interfaces
brctl addif ${bridge_name} ${viface1}
brctl addif ${bridge_name} ${viface2}

echo Bringing bridge up
ifconfig ${bridge_name} up

fi
