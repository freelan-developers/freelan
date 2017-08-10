Build instructions
==================

Third-party libraries
---------------------

FreeLAN depends on the following libraries:

- libcurl4
- boost
- OpenSSL
- iconv (Windows)

Generally Linux users can just use the binaries provided by their package manager, Mac OSX users can use brew but other platforms such as Windows may need to build these libraries explicitely.

To build the third-party libraries, you can use [teapot](https://github.com/freelan-developers/teapot) (be sure to have `python 2.7` and `perl` installed). Check its [documentation](http://teapot-builder.readthedocs.org/en/latest/) for details, or just type the following command at the root of FreeLAN's repository:

> teapot build

This will build everything inside `third-party`. Once that is done, you can proceed with the next steps.

### Debian Linux

To install the required dependencies on Debian Linux (Or Ubuntu), type the following command:

> sudo apt-get install scons python libssl-dev libcurl4-openssl-dev libboost-system-dev libboost-thread-dev libboost-program-options-dev libboost-filesystem-dev libboost-iostreams-dev build-essential

### Mac OSX

To install the required dependencies on Mac OS, type the following commands: 

> /usr/bin/ruby -e "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install)"

> brew update

> brew install scons boost openssl

Building FreeLAN
----------------

FreeLAN is written in C++11. Make sure your C++ compiler is recent enough and supports this standard. On Linux for instance, you need at least `gcc >= 4.8` for things to go fine.

FreeLAN uses [scons](http://www.scons.org/) as its build system on POSIX-like systems (basically everything except Windows).

To build the main binary, just type:

> scons

To build everything in the directory (including samples), type instead:

> scons samples

To build then install everything into a specific directory, type instead:

> scons install prefix=/usr/local/

### Debugging

If the debug-level logging exposed with the `-d` parameter to freelan does not expose enough information to assist development or bug-finding, it is possible to enable additional debug information at build time with:

> scons all --mode=debug

Be aware that this will produce a significant amount of logging information and is not intended for production use.

### Windows

You will need Microsoft Visual Studio 2017 to compile freelan. All projects come with a `.vcxproj` file for all the targets (debug/release and x86/x64).

The root directory also contains a solution file (`.sln`) that references all the sub-projects.

The resulting binaries will be located in the [install](install) directory.

### Mac OSX

On Mac OSX, an additional SCons target exists to build the freelan installation package:

> scons package

The package will be generated at the root of the repository.
