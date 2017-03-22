#!/bin/bash

function finish {
  printf "\n Stopping camera \n "
 # sudo pkill -9 -f ~/ambient/camera.py #kill camera.py PID                (works!)
  printf "\n Stopping Dash7 sensor data \n "
 # sudo pkill -9 -f ~/pyd7a/examples/readNode.py #kill Dash7 sensor read   (does not work..)
  printf "\n Killing all python scripts \n"
  sudo pkill -f python #kill all python scripts                            (<--temp solution) 
  printf "\n Stopping rpi MEMS sensor data"
  printf "\nGoodbye! \n"
  exit
}

trap finish INT #execute 'finish' function on CTRL+C

printf "\n\n Starting camera!\n\n Check the TeamMMD openHAB at " 
printf `ifconfig eth0 2>/dev/null|awk '/inet addr:/ {print $2}'|sed 's/addr://'` 
sh ~/scripts/camStart.sh &

printf "\n\n Reading Dash7 sensor data \n\n "
sh ~/scripts/d7aSensorStart.sh &

printf "\n\n Reading rpi Mems data: \n\n "
sh ~/scripts/rpiMemsStart.sh 

