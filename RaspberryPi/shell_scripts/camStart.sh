#!/bin/bash
sudo python ~/ambient/camera.py

trap sudo pkill -f python SIGINT
