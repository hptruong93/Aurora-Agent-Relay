#!/bin/bash

modprobe -r mac80211_hwsim
ifconfig wlan0 down
ifconfig wlan0 up

if [ "$1" == "-d" ] || [ "$1" == "-terminate" ]; then
    sleep 1
else
    modprobe mac80211_hwsim radios=1
    ifconfig hwsim0 up
    ifconfig wlan1 up
    hostapd -dd /etc/hostapd/hostapd.conf
fi
