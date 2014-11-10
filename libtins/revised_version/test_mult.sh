#!/bin/bash

echo First aurora
bash bridge.sh eth0 a0 wlan0 a1 b0

sleep 1
echo Second aurora
bash bridge.sh eth0 a2 wlan1 a3 b1


sleep 1
echo First warp
bash bridge.sh wlan0 w0 eth1 w1 b2


#sleep 1
#echo Second warp
#bash bridge.sh wlan1 w2 eth1 w3 b3
