#!/bin/bash

screen -S client -d -m sh -c "{{ scripts_root }}/launch.sh ${1:-1}; exec bash"
