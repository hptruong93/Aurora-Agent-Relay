#!/bin/bash

radio_num=1
if [[ -n "$1" ]]; then
    radio_num=${1}
    shift
fi

killall hostapd
ifconfig wlan0 down
ifconfig wlan1 down
modprobe -r mac80211_hwsim

if [ "$1" == "-d" ] || [ "$1" == "-terminate" ]; then
    : 
else
    modprobe mac80211_hwsim radios=${radio_num}
    ifconfig hwsim0 up
    ifconfig wlan0 up
    ifconfig wlan1 up
    hostapd -dd /etc/hostapd/hostapd.conf
fi
