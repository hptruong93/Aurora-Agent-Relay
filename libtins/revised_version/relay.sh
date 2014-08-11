#!/bin/bash

#This only works as root
#Usage: bash relay.sh
#This will kill all hostapd, kill all screen to clean up.
#Then it will start hostapd with usr config file, and run screens to relay messages out to WARP
#-c or --clean option to only clean things up (not starting any hostapd or screen)
#-d or --hostapd to only clean things up and start hostapd (not starting any screen)
#-nd or --no-hostapd to not start hostapd (but still run the relay program)


if [ "$1" == "-nc" ] || [ "$1" == "--no-clean" ]; then
    shift 
else
    echo Cleaning up...
    killall hostapd
    killall screen
fi

echo Setting up environment...
ifconfig hwsim0 up
ifconfig wlan0 up
ifconfig eth1 up

if [ "$1" == "-b" ] || [ "$1" == "--bridge" ]; then
    echo Remove all veth
    killall vethd

    echo Add veths
    vethd -v veth0 -e eth0
    ifconfig veth0 up

    vethd -v vwlan0 -e wlan0
    ifconfig wlan0 up
    
    echo Add bridge
    ifconfig linux-br down
    brctl delbr linux-br
    
    brctl addbr linux-br
    brctl addif linux-br veth0
    brctl addif linux-br vwlan0

    ifconfig linux-br up
    shift
fi

if [ "$1" == "-c" ] || [ "$1" == "--clean" ]; then
    shift 
else
    if [ "$1" == "-nd" ] || [ "$1" == "--no-hostapd" ]; then
        shift
    else
        echo Running hostapd...
        screen -S hostapd -d -m hostapd -dd /etc/hostapd/hostapd.conf
    fi

    if [ "$1" == "-d" ] || [ "$1" == "--hostapd" ]; then
        shift
    else

        echo Relay from hwsim0 to eth1
        screen -S fromhw -d -m ./mon_to_warp.out hwsim0 eth1

        echo Relay from mon.wlan0 to eth1
        screen -S frommon -d -m ./mon_to_warp.out mon.wlan0 eth1

        echo Relay from eth1 to wlan interface 
        screen -S topc -d -m ./warp_to_wlan.out eth1 mon.wlan0

	echo Relay from wlan0 to eth1
        screen -S fromwlan -d -m ./wlan_to_warp.out wlan0 eth1

    fi
fi
