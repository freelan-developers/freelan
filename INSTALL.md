Installation
============

To deploy FreeLAN build tools, you just have to add this directory to your PYTHONPATH.

Libraries
=========

FreeLAN related libraries are based on several libraries which you have to build first.

Explaining exactly how to compile those libraries should be out of the scope of these tools but since we are nice, here is quick-guide to get started:

Boost
-----

First download Boost from [its official website](http://www.boost.org/) and decompress the archive.

To build the Boost::Python library, you will also require Python.

### Windows 32/64 bits

Open a console to the decompressed Boost directory and type:

> .\bootstrap.bat

To build the build tools.

Then, depending on your target platform:

#### MinGW

`gcc` must be in your PATH for this command to work.

For 32 bits:

> .\b2.exe install toolset=gcc --prefix=C:\Boost

For 64 bits:

> .\b2.exe install toolset=gcc address-model=64 --prefix=C:\Boost

#### Microsoft Visual Studio

You must run this command from a "Visual Studio command prompt".

For 32 bits:

> .\b2.exe install toolset=msvc --prefix=C:\Boost-VC

For 64 bits:

> .\b2.exe install toolset=msvc address-model=64 --prefix=C:\Boost-VC

This will build and install all Boost static libraries, in both release and debug configurations.

Whatever the target platform, you may of course adapt the `--prefix` to whatever you like.

OpenSSL
-------

To build OpenSSL, you need a working [Perl installation](http://www.perl.org/).

Download OpenSSL from [its official website](http://www.openssl.org/) and decompress the archive.

### Windows 32/64 bits

#### MinGW

You must run these commands from a MSys console.

For 32 bits:

> perl Configure mingw no-shared no-asm --prefix=/c/OpenSSL

For 64 bits:

> perl Configure mingw64 no-shared no-asm --prefix=/C/OpenSSL-x64

Then:

> make
> make install

Note that this will compile OpenSSL in static mode.

If at some point you get a "make (e=2):" error, ensure you don't have another "make.exe" in your PATH or just type `/bin/make` instead of `make`.

#### Microsoft Visual Studio

You must run these commands from a "Visual Studio command prompt".

For 32 bits:

> perl Configure VC-WIN32 no-asm --prefix=C:\OpenSSL-VC
> ms\do_ms

For 64 bits:

> perl Configure VC-WIN64A --prefix=C:\OpenSSL-VC-x64
> ms\do_win64a

Then:

> nmake -f ms\ntdll.mak
> nmake -f ms\ntdll.mak install

To build in shared mode or:

> nmake -f ms\nt.mak
> nmake -f ms\nt.mak install

To build in static mode.
