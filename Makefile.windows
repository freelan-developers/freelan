# This Makefile is targeted for nmake.exe (on Windows).
#
# To compile on Linux or Mac OSX, use `scons` (SConstruct).

default: build

build: build_x64 build_x86

clean: clean_x64 clean_x86

build_x64:
	msbuild freelan-all.sln /p:Configuration=Release /p:Platform=x64 /t:Build

build_x86:
	msbuild freelan-all.sln /p:Configuration=Release /p:Platform=Win32 /t:Build

clean_x64:
	msbuild freelan-all.sln /p:Configuration=Release /p:Platform=x64 /t:Clean

clean_x86:
	msbuild freelan-all.sln /p:Configuration=Release /p:Platform=Win32 /t:Clean
