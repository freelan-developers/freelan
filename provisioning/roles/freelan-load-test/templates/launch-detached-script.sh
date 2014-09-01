#!/bin/bash

screen -S {{ item.name }} -d -m {{ scripts_root }}/{{ item.name }}.sh
