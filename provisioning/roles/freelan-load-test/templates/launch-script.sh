#!/bin/bash

sudo {{ sources_root }}/install/bin/freelan -c {{ nodes_config_root }}/{{ item.name }}.cfg -f -d -t {{ threads_count }}
