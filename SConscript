"""
The main SConscript file.
"""

name = 'fscp'
major = '2'
minor = '1'
libraries = []

# You should not need to modify anything below this line

Import('env')

env = env.Clone()

import os, sys

from freelan.buildtools import LibraryProject

if sys.platform.startswith('win32'):
    libraries.append('cryptoplus_static')
else:
    libraries.append('cryptoplus')

libraries.append('boost_system')
libraries.append('boost_date_time')

if sys.platform.startswith('win32'):

    if env['CC'] == 'gcc':
        libraries.append('crypto')
        env['CXXFLAGS'].append('-DBOOST_THREAD_USE_LIB')
        env['CXXFLAGS'].append('-DBOOST_USE_WINDOWS_H')
        env['CXXFLAGS'].append('-D_WIN32_WINNT=0x0501')
    else:
        libraries.append('libeay32')
        libraries.append('advapi32')
        libraries.append('shell32')

        env['CXXFLAGS'].append('/DBOOST_THREAD_USE_LIB')
        env['CXXFLAGS'].append('/DBOOST_USE_WINDOWS_H')
        env['CXXFLAGS'].append('/D_WIN32_WINNT=0x0501')

    libraries.append('ws2_32')
    libraries.append('gdi32')
    libraries.append('iphlpapi')
else:
    libraries.append('pthread')
    libraries.append('crypto')

project = LibraryProject(Dir('.'), name, major, minor, libraries, Glob('src/*.cpp'))

build = env.FreelanProject(project)
install = env.FreelanProjectInstall(project)
documentation = env.FreelanProjectDocumentation(project)
indent = env.FreelanProjectIndent(project)
samples = env.SConscript('samples/SConscript', exports = 'env project')

targets = {
    'build': build,
    'install': install,
    'documentation': documentation,
    'indent': indent,
    'samples': samples,
}

Return('targets')
