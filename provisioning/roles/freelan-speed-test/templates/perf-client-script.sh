#!/bin/bash

iperf -c {{ item.ipv4_address }}
