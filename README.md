Freelan build tools
===================

A set of Python scripts to automate builds of FreeLAN related libraries.

Requirements
------------

These tools require Python and SCons to work properly.

You may get Python from [www.python.org](http://www.python.org) and SCons from [www.scons.org](http://www.scons.org).

Rationale
---------

Most FreeLAN related projects have the same architecture and are built the same way. That is, instead of having similar build scripts duplicated for each library, we decided it was better to have an unique method for that.

Every time the Freelan build tools are improved, every other library can benefit from it.

The tools can build projects for Windows, Linux Mac OSX and probably any other UNIX based operating system.

They also supports several compiler toolchains, especially for Windows when you can build everything using either MinGW or Microsoft Visual Studio.

Install
-------

To install Freelan build tools, run the following command (probably as root):

> ./setup.py install

How to use Freelan build tools
------------------------------

Freelan build tools are based on SConstruct and thus, building a project is as simple as typing:

> scons

For libraries projects, you may also build the samples using the following command:

> scons samples

You can also install the library/program typing:

> scons install

You may specify an installation prefix with:

> scons install --prefix=C:\freelan

> scons install --prefix=/usr/local/

### Build options

You may also customize the build with different options.

#### Specify the compiler toolchain to use (Windows only)

Example:

> scons toolset=default

> scons toolset=mingw

If the option is not specified, `default` is assumed.

#### Specify the release mode

> scons mode=release

> scons mode=debug

If the option is not specified, `release` is assumed.

#### Specify the target architecture

> scons arch=x86

> scons arch=x64

> scons arch=...

If the option is not specified, the current platform architecure is assumed.

#### Specify the library directory

> scons libdir=lib

If the option is not specified, `lib` is assumed.

#### Specify the binary directory

> scons bindir=bin

If the option is not specified, `bin` is assumed.

### Environment variables

Most parameters and options defaults can also be changed using environment variables.

Here is the list of the supported variables:

- `FREELAN_BUILD_PREFIX`, default for the `--build-prefix` parameter.
- `FREELAN_MINGW_BUILD_PREFIX`, Windows only. A default that, if defined, superseeds `FREELAN_BUILD_PREFIX` when MinGW is used as compiler toolchain.
- `FREELAN_MSVC_BUILD_PREFIX`, Windows only. A default that, if defined, superseeds `FREELAN_BUILD_PREFIX` when Microsoft Visual Studio is used as compiler toolchain.
- `FREELAN_INSTALL_PREFIX`, default for the `--prefix` parameter.
- `FREELAN_MINGW_INSTALL_PREFIX`, Windows only. A default that, if defined, superseeds `FREELAN_INSTALL_PREFIX` when MinGW is used as compiler toolchain.
- `FREELAN_MSVC_INSTALL_PREFIX`, Windows only. A default that, if defined, superseeds `FREELAN_INSTALL_PREFIX` when Microsoft Visual Studio is used as compiler toolchain.
- `FREELAN_TOOLSET`, default for the `toolset` option.
- `FREELAN_STATIC_SUFFIX`, default for the `static_suffix` option. This suffix is added to the static library name.
- `FREELAN_MODE`, default for the `mode` option.
- `FREELAN_ARCH`, default for the `arch` option.
- `FREELAN_LIBDIR`, default for the `libdir` option.
- `FREELAN_BINDIR`, default for the `bindir` option.
- `FREELAN_MINGW_RELEASE_BOOST_PREFIX`, Windows only. The Boost library prefix to use for release builds with MinGW.
- `FREELAN_MINGW_DEBUG_BOOST_PREFIX`, Windows only. The Boost library prefix to use for debug builds with MinGW. If not specified, `FREELAN_MINGW_RELEASE_BOOST_PREFIX` is used for both release and debug builds.
- `FREELAN_MINGW_RELEASE_BOOST_SUFFIX`, Windows only. The Boost library suffix to use for release builds with MinGW.
- `FREELAN_MINGW_DEBUG_BOOST_SUFFIX`, Windows only. The Boost library suffix to use for debug builds with MinGW. If not specified, `FREELAN_MINGW_RELEASE_BOOST_SUFFIX` is used for both release and debug builds.
- `FREELAN_MSVC_RELEASE_BOOST_PREFIX`, Windows only. The Boost library prefix to use for release builds with Microsoft Visual Studio.
- `FREELAN_MSVC_DEBUG_BOOST_PREFIX`, Windows only. The Boost library prefix to use for debug builds with Microsoft Visual Studio. If not specified, `FREELAN_MSVC_RELEASE_BOOST_PREFIX` is used for both release and debug builds.
- `FREELAN_MSVC_RELEASE_BOOST_SUFFIX`, Windows only. The Boost library suffix to use for release builds with Microsoft Visual Studio.
- `FREELAN_MSVC_DEBUG_BOOST_SUFFIX`, Windows only. The Boost library suffix to use for debug builds with Microsoft Visual Studio. If not specified, `FREELAN_MSVC_RELEASE_BOOST_SUFFIX` is used for both release and debug builds.

The rationale for the prefix/suffix variables is that under Windows, Boost by default decorates its libraries names with the compiler name and the Boost version. We designed the Freelan build tools so that they match the defaults of the libraries we depend on.

You may also define SCons specific environment variables by prefixing them with `FREELAN_ENV_`.

Example: To use `g++44` instead of `g++` as the `CXX` environment variable, define `FREELAN_ENV_CXX` to `g++44`.

Sample configuration
--------------------

Here is a configuration setup I use in my .bashrc (on Linux) to isolate freelan builds from the operating system:

    # WORKSPACE_ROOT is a directory which contains include, lib and bin sub-directories.
    export WORKSPACE_ROOT="${PROJECTS_ROOT}/workspace"
    
    # This allows me to type ldconfig as a non-root user.
    alias ldconfig="ldconfig -r ${WORKSPACE_ROOT}"
    
    export FREELAN_INSTALL_PREFIX="${WORKSPACE_ROOT}"
    export CPLUS_INCLUDE_PATH="${FREELAN_INSTALL_PREFIX}/include:${CPLUS_INCLUDE_PATH}"
    export C_INCLUDE_PATH="${FREELAN_INSTALL_PREFIX}/include:${C_INCLUDE_PATH}"
    export LIBRARY_PATH="${FREELAN_INSTALL_PREFIX}/lib:${LIBRARY_PATH}"
    export LD_LIBRARY_PATH="${FREELAN_INSTALL_PREFIX}/lib:${LD_LIBRARY_PATH}"
    export PATH="${FREELAN_INSTALL_PREFIX}/bin:${PATH}"

Reusability
-----------

While Freelan build tools are not meant to be used for non FreeLAN related project, you might however use them (as-is, or modified) to build your own projects.

If you do so and think about an improvement of which we could also benefit, feel free to suggest changes ! The preferred way is to fork the project on GitHub, then to do a pull-request.

Does the project have a website ?
---------------------------------

You may find the [git repository](https://github.com/freelan-developers/freelan-buildtools) on github.
