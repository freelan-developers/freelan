"""The SConstruct file"""

name = 'freelan'
major = '1'
minor = '0'
libraries = []

# You should not need to modify anything below this line

import os, sys

from freelan.build_tools import ProgramProject, Environment

env = Environment(ENV = os.environ.copy(), ARGUMENTS = ARGUMENTS)

libraries.append('freelan')
libraries.append('asiotap')
libraries.append('fscp')
libraries.append('cryptoplus')
libraries.append('boost_system')
libraries.append('boost_thread')
libraries.append('boost_program_options')
libraries.append('ssl')
libraries.append('crypto')

if sys.platform.startswith('win32'):

    if env['CC'] == 'gcc':
        libraries.append('crypto')
        env['CXXFLAGS'].append('-DBOOST_THREAD_USE_LIB')
        env['CXXFLAGS'].append('-DBOOST_USE_WINDOWS_H')
        env['CXXFLAGS'].append('-D_WIN32_WINNT=0x0501')
        env['CXXFLAGS'].append('-D_WIN32_IE=0x0500')
    else:
        libraries.append('libeay32')
        libraries.append('advapi32')
        libraries.append('shell32')

        env['CXXFLAGS'].append('/DBOOST_THREAD_USE_LIB')
        env['CXXFLAGS'].append('/DBOOST_USE_WINDOWS_H')
        env['CXXFLAGS'].append('/D_WIN32_WINNT=0x0501')
        env['CXXFLAGS'].append('/D_WIN32_IE=0x0500')

    libraries.append('ws2_32')
    libraries.append('gdi32')
    libraries.append('iphlpapi')
else:
    libraries.append('pthread')

    if sys.platform.startswith('linux2'):
        libraries.append('rt')

project = ProgramProject(name, major, minor, libraries)

env.FreelanProject(project)
