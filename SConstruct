##
# libcryptopen build file.
#

import os, sys, platform

sys.path.append(os.path.abspath(os.path.join('.', 'scons')))
import flenv

# Sources and name of the systools library
lib_target = 'cryptopen'
lib_major = '1'
lib_minor = '0'
lib_sources = Glob("src/*.cpp")
lib_external_includes = Glob("include/" + lib_target + "/*.hpp")
lib_cpppath = flenv.cpppath + ['include', 'include/' + lib_target]
lib_cxxflags = flenv.cxxflags + ['-Wshadow']
lib_libpath = flenv.libpath + []
lib_libs = flenv.libs + []
lib_shlinkflags = ['-shared']

lib_includes = lib_external_includes + Glob("include/*.hpp")

# MAC OS specific adjustments
if sys.platform == 'darwin':
	lib_shlinkflags.append('-arch')
	lib_shlinkflags.append('i386')
	lib_shlinkflags.append('-arch')
	lib_shlinkflags.append('x86_64')
	lib_shlinkflags.append('-single_module')
	lib_shlinkflags.append('-undefined')
	lib_shlinkflags.append('dynamic_lookup')
# Other OSes don't have the same options
else:
	if sys.platform != 'win32':
		lib_shlinkflags.append('-Wl,-soname,lib' + lib_target + '.so.' + lib_major)
	else:
		lib_shlinkflags.append('-Wl,--output-def,lib\\' + lib_target + '.def')

# Build with debug symbols or not
if ARGUMENTS.get('mode', 0) == 'debug':
	lib_cxxflags.append('-g')
else:
	lib_cxxflags.append('-DNDEBUG')

# Allow to output 32 bit library from 64 bit OS and vice-versa.
# It needs gcc-multilib and g++-multilib packages installed.
# For MS Windows, it needs gcc >= 4.5.0 to support this feature.
# For Mac OS X (darwin) we compile a universal library anyway
if sys.platform != 'darwin':
	if ARGUMENTS.get('arch', 0) == '32':
		lib_cxxflags.append('-m32')
		lib_shlinkflags.append('-m32')
	elif ARGUMENTS.get('arch', 0) == '64':
		lib_cxxflags.append('-m64')
		lib_shlinkflags.append('-m64')

# Installation directory
install_path = flenv.GetVar('install_path')

# Create an environment
env = Environment(
		ENV = os.environ.copy(),
		tools = [flenv.target_platform, 'doxygen', 'astyle'],
		toolpath = ['.', './scons'],
		variables = flenv.LoadVariables(),
		CXXFLAGS = lib_cxxflags,
		CPPPATH = lib_cpppath,
		LIBPATH = lib_libpath,
		SHLINKFLAGS = lib_shlinkflags)

# Static library
lib_static_sources = env.StaticObject(lib_sources)
lib_static = env.StaticLibrary(target = 'lib/' + lib_target + '_static', source = lib_static_sources, LIBS = lib_libs)

# Shared library
lib_shared_sources = env.SharedObject(lib_sources)
lib_shared = env.SharedLibrary(target = 'lib/' + lib_target, source = lib_shared_sources, LIBS = lib_libs)

# We filter out the dll from the shared library targets
# This only has an effect on Windows
lib_shared_dll = [elem for elem in lib_shared if (str(elem)[-3:] == 'dll')]
lib_shared_not_dll = [elem for elem in lib_shared if (str(elem)[-3:] != 'dll')]

# Install script
env.Install(dir = os.path.join(install_path, 'bin'), source = lib_shared_dll)
env.Install(dir = os.path.join(install_path, 'lib'), source = lib_shared_not_dll)
env.Install(dir = os.path.join(install_path, 'lib'), source = lib_static)
env.Install(dir = os.path.join(install_path, 'include', lib_target), source = lib_external_includes)

if sys.platform != 'win32':
	env.Install(dir = os.path.join(install_path, 'lib', 'pkgconfig'), source = lib_pkgconfig)

# Doxygen
doxygen = env.Doxygen('doxyfile')
AlwaysBuild(doxygen)

# A-Style
astyle = env.AStyle(lib_sources + lib_includes);

# Alias for targets
env.Alias('static', [lib_static])
env.Alias('shared', [lib_shared])
env.Alias('build', [lib_static, lib_shared])
env.Alias('install', [install_path])
env.Alias('doc', doxygen)
env.Alias('indent', astyle)
env.Alias('all', ['build', 'doc'])

# Save the configuration
flenv.SaveVariables(env)

# Help documentation
Help("""
Type: 'scons build' to build cryptopen library.
      'scons install' to install libraries and include files on the system.
      'scons doc' to build documentation (doxygen).
      'scons all' to build libraries and documentation.
      'scons -c' to cleanup object and libraries files.
      'scons -c install' to uninstall libraries and include files.
      'scons -c doc' to cleanup documentation files.
      'scons -c all' to cleanup libraries and documentation files.
      \n
      If scons is called without parameters, the default target is "build".
""")

# Default target when running scons without arguments
Default('build')
