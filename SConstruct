"""The SConstruct file"""

name = 'fscp'
major = '1'
minor = '0'
libraries = []

# You should not need to modify anything below this line

import os, sys

from freelan.build_tools import LibraryProject, Environment

env = Environment(ENV = os.environ.copy(), ARGUMENTS = ARGUMENTS)

libraries.append('cryptoplus')
libraries.append('boost_system')
libraries.append('boost_thread')

if sys.platform.startswith('win32'):
    libraries.append('ws2_32')
    libraries.append('gdi32')
    libraries.append('iphlpapi')

    if env['CC'] == 'gcc':
        env['CXXFLAGS'].append('-DBOOST_THREAD_USE_LIB')
        env['CXXFLAGS'].append('-DBOOST_USE_WINDOWS_H')
        env['CXXFLAGS'].append('-D_WIN32_WINNT=0x0501')
    else:
        libraries.append('advapi32')
        libraries.append('shell32')

        env['CXXFLAGS'].append('/DBOOST_THREAD_USE_LIB')
        env['CXXFLAGS'].append('/DBOOST_USE_WINDOWS_H')
        env['CXXFLAGS'].append('/D_WIN32_WINNT=0x0501')
else:
    libraries.append('pthread')

    if sys.platform.startswith('linux2'):
        libraries.append('rt')

project = LibraryProject(name, major, minor, libraries)

env.FreelanProject(project)

samples = env.SConscript('samples/SConscript', exports = 'env project')
env.Alias('samples', samples)
