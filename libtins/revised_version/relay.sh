#!/bin/bash

#This only works as root
#Usage: bash relay.sh [-c] [-d hostapd_dir] wlan_interface ethernet_interface [bssid]

if [ "$1" == "-c" ] || [ "$1" == "--clean" ]; then
    echo Cleaning up...
    killall hostapd
    killall screen
    shift
fi

if [ "$1" == "-d" ] || [ "$1" == "--hostapd" ]; then
    echo Running hostapd...
    shift
    if [[ -n "$1" ]]; then
        hostapd_dir=${1}
        screen -S hostapd-at-${hostapd_dir} -d -m hostapd -dd ${hostapd_dir}
        shift
    else
        echo "Missing hostapd directory... Exiting..."
        exit 1
    fi
fi

if [[ -n "$1" ]]; then
    wlan_iface=${1}
    shift
else
    echo "Missing wlan interface... Exiting..."
    exit 1
fi

if [[ -n "$1" ]]; then
    eth_iface=${1}
    shift
else
    echo "Missing ethernet interface... Exiting..."
    exit 1
fi

bssid="40:d8:55:04:22:84"
if [[ -n "$1" ]]; then
    bssid=${1}
    shift
else
    echo "Missing bssid... Using default bssid 40:d8:55:04:22:84"
    bssid="40:d8:55:04:22:84"
fi

echo Setting up environment with ${wlan_iface} to ${eth_iface}, bssid is ${bssid}
ifconfig hwsim0 up
ifconfig ${wlan_iface} up
ifconfig ${eth_iface} up

echo Relay from hwsim0 to ${eth_iface}
screen -S frommon${wlan_iface} -d -m ./mon_to_warp.out hwsim0 ${eth_iface} ${bssid}

echo Relay from ${eth_iface} to wlan interface 
screen -S to${wlan_iface} -d -m ./warp_to_wlan.out ${eth_iface} mon.${wlan_iface} ${wlan_iface} ${bssid}

echo Relay from ${wlan_iface} to ${eth_iface}
screen -S from${wlan_iface} -d -m ./wlan_to_warp.out ${wlan_iface} ${eth_iface} ${bssid}
