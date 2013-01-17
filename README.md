freelan-all
===========

A super-project that includes all FreeLAN related projects and provides scripts to generate installers.

For help regarding the configuration, take a look at the [wiki](https://github.com/freelan-developers/freelan-all/wiki).

Libraries
---------

It contains the following projects:

 - [libcryptoplus](https://github.com/freelan-developers/libcryptoplus) - A C++ wrapper around the OpenSSL cryptographic library.
 - [libiconvplus](https://github.com/freelan-developers/libiconvplus) - A C++ wrapper around the iconv library.
 - [libfscp](https://github.com/freelan-developers/libfscp) - The FreeLAN Secure Channel protocol and its C++ implementation.
 - [libasiotap](https://github.com/freelan-developers/libasiotap) - A C++ library to control virtual network interfaces also known as "TAP adapters".
 - [libfreelan](https://github.com/freelan-developers/libfreelan) - The FreeLAN C++ library.
 - [freelan](https://github.com/freelan-developers/freelan) - The FreeLAN binary.
 - [freelan-server](https://github.com/freelan-developers/freelan-server) - The FreeLAN configuration server.
 - [libkfather](https://github.com/freelan-developers/libkfather) - A C++ JSON parser/producer with a terrible name.

To fetch those from their respective repositories, issue the following commands:

> git submodule init

> git submodule update

And, to get the last revision for each repository:

> git submodule foreach git checkout master

Building
--------

You may setup quickly a building environment by calling the appropriate script at `tools/<your operating system>/setup-environment`.

If no script exists for your operating system, read the instructions in `freelan-buildtools/README.md`.

This directory contains a `SConstruct` file which uses the [FreeLAN builds tools](https://github.com/freelan-developers/freelan-buildtools) to build the contained projects.

The default target (`freelan`) builds and installs all the necessary files for FreeLAN and FreeLAN itself to the environment specified `FREELAN_INSTALL_PREFIX` (or a OS-dependent default prefix if none is specified).

To do that, just type:

> scons

To build everything in the directory (all sub-projects), type instead:

> scons build

To build then install everything from the directory (all sub-projects), type instead:

> scons install

Graphical User Interface
------------------------

A graphical user interface exists (or will exist soon enough :D) and is available here:

 - [freelan-gui](https://github.com/freelan-developers/freelan-gui) - A Graphical User Interface for FreeLAN.

Packaging
----------

See the `README.md` file into the `packaging` directory.

Does the project have a website ?
---------------------------------

You may find the [git repository](https://github.com/freelan-developers/freelan-all) on github.
