#!/bin/bash

modprobe -r mac80211_hwsim
modprobe mac80211_hwsim radios=1
ifconfig hwsim0 up
ifconfig wlan1 up
hostapd -dd /etc/hostapd/hostapd.conf
