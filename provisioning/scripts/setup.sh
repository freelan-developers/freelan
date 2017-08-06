#!/bin/sh

echo "Provisioning virtual machine..."

# Ubuntu vagrant box does not come with python and aptitude which are required
# for ansible use
apt-get -y install python aptitude

