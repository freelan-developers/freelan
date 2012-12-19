Installation for Android
========================

The freelan build-tools can be used for Android cross-compilation builds.

You may want to read the instructions inside the `INSTALL.md` file before reading this file.

Libraries
=========

To cross-compile for Android, you need the Android NDK which is available [here](http://developer.android.com/tools/sdk/ndk/index.html).

While cross-compilation could theoricaly be done under any operating system, please note that this guide assumes that a Linux platform is used. If you know how to adapt this guide for Windows, please feel free to submit a pull-request.

For the rest of this document, we will assume you have installed the NDK to the following directory:

`export NDK_ROOT=/tmp/ndk`

Boost
-----

Building Boost for Android is not an simple task and requires patching the source files. As this is annoying, boring and error-prone, some guys made a script to ease the whole process, and you may want to thank them for that.

Anyway, the script is available [here](https://github.com/MysticTreeGames/Boost-for-Android).
