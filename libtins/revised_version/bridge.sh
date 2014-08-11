#!/bin/bash

iface1=${1}
iface1=${2}
bridge=${3}

echo Clean up interfaces
ifconfig v${iface1} down
ifconfig v${iface2} down
killall vethd


echo Setting up interfaces
vethd -v v${iface1} -e ${iface1}
ifconfig v${iface1} up

vethd -v v${iface2} -e ${iface2}
ifconfig v${iface2} up


echo Clean up bridge
ifconfig ${bridge} down
brctl delbr ${bridge}

echo Creating new bridge
brctl addbr ${bridge}
brctl addif ${bridge} v${iface1}
brctl addif ${bridge} v${iface2}
ifconfig ${bridge} up
