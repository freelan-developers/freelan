freelan-all
===========

A super-project that includes all FreeLAN related projects and provides scripts to generate installers.

For help regarding the configuration, take a look at the [wiki](https://github.com/freelan-developers/freelan-all/wiki).

Binaries
--------

It contains the following binaries:

- freelan - The FreeLAN binary (acts as a service on Windows and as a daemon on Linux/Mac OSX)

Libraries
---------

It contains the following projects:

 - cryptoplus - A C++ wrapper around the OpenSSL cryptographic library.
 - iconvplus - A C++ wrapper around the iconv library.
 - fscp - The FreeLAN Secure Channel protocol and its C++ implementation.
 - asiotap - A C++ library to control virtual network interfaces also known as "TAP adapters".
 - freelan - The FreeLAN C++ library.
 - kfather - A C++ JSON parser/producer with a terrible name.

Building
--------

### Third-party

The build relies on several third-parties. To build those, install the Python command `teapot` using the following command:

> pip install teapot

Once you have `teapot` in your `PATH` (should be the case automatically), just type:

> teapot build

To build everything inside `third-party`. Once that is done, you can proceed with the next steps.

### Common code

This directory contains a `SConstruct` file to build the contained projects.

The default target (`install`) builds and installs all the main FreeLAN binary.

To do that, just type:

> scons

To build everything in the directory (all samples), type instead:

> scons samples

To build then install everything into a specific directory, type instead:

> scons install --prefix=/usr/local/

### Windows

You will need Microsoft Visual Studio 2013 to compile freelan. All projects come with a `.vcxproj` file for all the targets (debug/release and x86/x64).

The root directory also contains a solution file (`.sln`) that references all the sub-projects.

Graphical User Interface
------------------------

A graphical user interface is planned but nothing is made public yet. You will just have to be patient :)

Packaging
---------

See the `README.md` file into the `packaging` directory.

Does the project have a website ?
---------------------------------

You may find the [git repository](https://github.com/freelan-developers/freelan-all) on github.
