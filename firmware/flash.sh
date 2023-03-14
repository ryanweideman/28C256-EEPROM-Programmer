#!/bin/bash

sudo chmod a+rw /dev/ttyACM0
arduino-cli upload -p /dev/ttyACM0 --fqbn arduino:avr:mega firmware.ino
