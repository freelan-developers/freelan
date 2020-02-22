# Sailfish OS packaging

## Install Sailfish OS SDK

Follow https://sailfishos.org/wiki/Application_SDK_Installation

## VM setup

Launch VirtualBox and open MerSDK VM configuration, increase RAM to 2048 MB and
also increase processor number to speed-up (cross-)compilation.

Start the MerSDK VM and connect to it:

`ssh -p 2222 -i ~/SailfishOS/vmshare/ssh/private_keys/engine/mersdk
mersdk@localhost`

## Setup environment

On the VM ssh shell, install some dependencies.

For x86 target:

`sb2 -t SailfishOS-i486 -m sdk-install -R zypper install scons boost-devel
openssl-devel libcurl-devel boost-thread help2man`

For ARM target:

`sb2 -t SailfishOS-armv7hl -m sdk-install -R zypper install scons boost-devel
openssl-devel libcurl-devel boost-thread help2man`

Retrieve sources:

`git clone https://github.com/freelan-developers/freelan.git && cd freelan`

## Build FreeLAN

For x86 target:

`sb2 -t SailfishOS-i486 scons --upnp=no -j2`

For ARM target:

`sb2 -t SailfishOS-armv7hl scons --upnp=no -j2`

## Package

For x86 target:

`mb2 -t SailfishOS-i486 -s packaging/rpm/freelan-sailfishos.spec build`

For ARM target:

`mb2 -t SailfishOS-armv7hl -s packaging/rpm/freelan-sailfishos.spec build`

## Installation

The generated RPM will be located in ./RPMS/. Upload it to your device (by SSH,
SD card, ...) and run the following to install:

```
pkcon refresh
pkcon install-local /path/to/freelan-2.2-1.armv7hl.rpm
```

