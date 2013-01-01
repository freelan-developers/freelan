"""
The main SConscript file.
"""

name = 'asiotap'
major = '1'
minor = '0'
libraries = []

# You should not need to modify anything below this line

Import('env')

env = env.Clone()

import os, sys

from freelan.buildtools import LibraryProject, Environment

libraries.append('boost_system')

if sys.platform.startswith('win32'):
    libraries.append('ws2_32')
    libraries.append('gdi32')
    libraries.append('iphlpapi')

    if env['CC'] == 'gcc':
        env['CXXFLAGS'].append('-DBOOST_THREAD_USE_LIB')
        env['CXXFLAGS'].append('-DBOOST_USE_WINDOWS_H')
        env['CXXFLAGS'].append('-DTAP_ID=\\"%s\\"' % os.environ.get('FREELAN_TAP_ID', 'tap0901'))
        env['CXXFLAGS'].append('-D_WIN32_WINNT=0x0501')
    else:
        libraries.append('advapi32')
        libraries.append('shell32')

        env['CXXFLAGS'].append('/DBOOST_THREAD_USE_LIB')
        env['CXXFLAGS'].append('/DBOOST_USE_WINDOWS_H')
        env['CXXFLAGS'].append('/DTAP_ID=\\"%s\\"' % os.environ.get('FREELAN_TAP_ID', 'tap0901'))
        env['CXXFLAGS'].append('/D_WIN32_WINNT=0x0501')
else:
    libraries.append('pthread')

    if sys.platform.startswith('linux2'):
        libraries.append('rt')

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
