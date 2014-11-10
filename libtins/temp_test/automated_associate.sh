#!/bin/bash

socket=${1}
echo Socket is ${socket}

echo Adding first slice 
python datapath_manager.py add ${socket} tb w0 eth1

echo Adding second slice
python datapath_manager.py add ${socket} tb w2 eth1

python datapath_manager.py show ${socket} tb

if [ -n "$2" ]; then
    station = ${2}
    echo Station mac address is ${station}
    echo Associating station to first slice
    python datapath_manager.py associate ${socket} tb w0 eth1 ${station}
    
    echo 
    echo
    echo Associating station to second slice
    python datapath_manager.py associate ${socket} tb w2 eth1 ${station}
fi
