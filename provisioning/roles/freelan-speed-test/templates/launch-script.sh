#!/bin/bash

sudo {{ sources_root }}/install/bin/freelan -c {{ config_path }} -f -d -t ${1:-1}
