#!/bin/bash

iperf -c {{ item.vpn_ipv4_address }}
