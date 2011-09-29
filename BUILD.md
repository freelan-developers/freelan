Build instructions
==================

To build this library, you need [Python](http://www.python.org), [SCons](http://www.scons.org) and [FreeLAN build tools](https://github.com/ereOn/freelan-build_tools).

Then, just type:

> scons

To build the library :)

You may combine different options to alter the build:

Use Visual Studio on Windows
----------------------------

Visual Studio is the default for Windows. You don't have to specify anything.

Use MinGW on Windows
--------------------

> scons toolset=mingw

Build a shared library instead of a static one
----------------------------------------------

> scons link=shared

Build a debug version
---------------------

> scons mode=debug

Change architecture
-------------------

> scons arch=x64
> scons arch=x86_64
> scons arch=i386
> scons arch=32
> ...

Clean the build
---------------

> scons -c [same options]
