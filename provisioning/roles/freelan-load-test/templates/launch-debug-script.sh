#!/bin/bash

sudo gdb {{ sources_root }}/build/debug/bin/freelan -ex 'run -c {{ nodes_config_root }}/{{ item.name }}.cfg -f -d -t {{ threads_count }}'
