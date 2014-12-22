#!/bin/bash

DURATION=1
NOMINAL_SPEED=`iperf -c {{ item.ipv4_address }} -f k -t ${DURATION} -y C | cut -f9 -d,`
VPN_SPEED=`iperf -c {{ item.vpn_ipv4_address }} -f k -t ${DURATION} -y C | cut -f9 -d,`
SPEED_RATIO=`expr \( ${VPN_SPEED} \* 100 \) / ${NOMINAL_SPEED}`

echo "Nominal speed: ${NOMINAL_SPEED} kbits/s"
echo "VPN speed: ${VPN_SPEED} kbits/s"
echo "Ratio: ${SPEED_RATIO}%"
