#!/bin/bash

screen -S perf -d -m sh -c "{{ scripts_root }}/perf-server.sh; exec bash"
