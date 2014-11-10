#!/bin/bash

if [ "$1" == "-s" ]; then
    echo Creating aurora0
    bash bridge.sh eth0 a0 wlan0 a1 b0

    echo Creating aurora1
    bash bridge.sh eth0 a2 wlan1 a3 b1

    echo Creating first pair w0 and w1
    vethd -v w0 -e wlan0
    ifconfig w0 up
    vethd -v w1 -e eth1
    ifconfig w1 up


    echo Creating second pair w2 and w3
    vethd -v w2 -e wlan1
    ifconfig w2 up
    #vethd -v w3 -e eth1
    #ifconfig w3 up


elif [ "$1" == "-c" ]; then
    bash bridge.sh -c a0 a1 a2 a3 w0 w1 w2 w3
    bash bridge.sh -cb b0 b1
    ifconfig tb down
    ovs-vsctl del-br tb
    ps -e | grep ovs
fi
