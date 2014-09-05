#!/bin/bash

#Usage: bash demo.sh

wait_for_input() {
        echo Proceeding to ${1}
        echo "Enter 1 to continue. 2 to exit"
        select yn in "Continue" "Exit"; do
        case $yn in
                Continue ) break;;
                Exit ) exit;;
        esac
        done
}

wait_for_input "Cleaning up..."
echo Big clean up...
killall hostapd
killall vethd

#Insmod mac80211 first
rmmod mac80211_hwsim

echo insmod mac80211_hwsim radios=2
insmod mac80211_hwsim radios=2

ifconfig wlan0 up
ifconfig wlan1 up
ifconfig hwsim0 up

#################################################################################
wait_for_input "Create one virtual wireless network"
#Start hostapd
echo Starting hostapd on wlan0
echo hostapd -dd /usr/hostapd0.conf
screen -S hostapd0 -d -m hostapd -dd /usr/hostapd0.conf

#Start connections:
echo Starting relay processes from hwsim interface to ethernet
bash relay.sh wlan0 eth1 40:d8:55:04:22:84

echo Network named test0 should be up
#################################################################################
wait_for_input "Connecting the virtual wireless network to the BCRL network"
echo Start bridging eth0 and wlan0
bash bridge.sh eth0 veth0-0 wlan0 vwlan0 linux-br0
#################################################################################

wait_for_input "Create another virtual wireless network"
#Start another hostapd
echo Starting hostapd on wlan1
echo hostapd -dd /usr/hostapd1.conf
screen -S hostapd1 -d -m hostapd -dd /usr/hostapd1.conf

#Start connections
echo Starting relay processes from hwsim interface to ethernet
bash relay.sh wlan1 eth1 40:d8:55:04:22:80

echo Network named test1 should be up
#################################################################################
wait_for_input "Connecting the virtual wireless network to the BCRL network"
echo Start bridging eth0 and wlan1
bash bridge.sh eth0 veth0-1 wlan1 vwlan1 linux-br1
