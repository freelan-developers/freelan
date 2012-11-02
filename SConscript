"""
The main SConscript file.
"""

name = 'freelan'

major = '1'
minor = '1'
libraries = []

# You should not need to modify anything below this line

Import('env')

env = env.Clone()

import os
import sys
import datetime

from freelan.buildtools import ProgramProject
import freelan.file_tools as file_tools

if sys.platform.startswith('win32') and env['CC'] != 'gcc':
    env['CXXFLAGS'].append('/DFREELAN_VERSION_MAJOR=%s' % major)
    env['CXXFLAGS'].append('/DFREELAN_VERSION_MINOR=%s' % minor)
    env['CXXFLAGS'].append(r"/DFREELAN_DATE=\"%s\"" % datetime.date.today().strftime('%a %d %b %Y'))
else:
    env['CXXFLAGS'].append('-DFREELAN_VERSION_MAJOR=%s' % major)
    env['CXXFLAGS'].append('-DFREELAN_VERSION_MINOR=%s' % minor)
    env['CXXFLAGS'].append(r"-DFREELAN_DATE=\"%s\"" % datetime.date.today().strftime('%a %d %b %Y'))

if sys.platform.startswith('win32'):
    libraries.append('freelan_static')
    libraries.append('asiotap_static')
    libraries.append('fscp_static')
    libraries.append('cryptoplus_static')
    libraries.append('kfather_static')
    libraries.append('iconvplus_static')
    libraries.append('iconv')
else:
    libraries.append('freelan')
    libraries.append('asiotap')
    libraries.append('fscp')
    libraries.append('cryptoplus')
    libraries.append('kfather')
    libraries.append('iconvplus')

libraries.append('boost_system')
libraries.append('boost_program_options')
libraries.append('boost_filesystem')
libraries.append('boost_date_time')
libraries.append('crypto')

if sys.platform.startswith('win32'):

    libraries.append('boost_thread')

    if env['CC'] == 'gcc':
        libraries.append('curl')
        libraries.append('ssl')
        libraries.append('crypto')
        env['CXXFLAGS'].append('-DBOOST_THREAD_USE_LIB')
        env['CXXFLAGS'].append('-DBOOST_USE_WINDOWS_H')
        env['CXXFLAGS'].append('-D_WIN32_WINNT=0x0501')
        env['CXXFLAGS'].append('-D_WIN32_IE=0x0500')
        env['CXXFLAGS'].append('-DUNICODE')
    else:
        libraries.remove('libcurl_a')
        libraries.remove('ssl')
        libraries.remove('crypto')
        libraries.append('libeay32')
        libraries.append('advapi32')
        libraries.append('shell32')

        env['CXXFLAGS'].append('/DBOOST_THREAD_USE_LIB')
        env['CXXFLAGS'].append('/DBOOST_USE_WINDOWS_H')
        env['CXXFLAGS'].append('/D_WIN32_WINNT=0x0501')
        env['CXXFLAGS'].append('/D_WIN32_IE=0x0500')
        env['CXXFLAGS'].append('/DUNICODE')

    libraries.append('ws2_32')
    libraries.append('gdi32')
    libraries.append('iphlpapi')
else:
    libraries.append('pthread')
    libraries.append('curl')
    libraries.append('ssl')
    libraries.append('crypto')

common_source_files = Glob('src/common/*.cpp')
env.VariantDir('build/service', 'src')
env.VariantDir('build/daemon', 'src')

source_files = Glob('src/*.cpp')

if sys.platform.startswith('win32'):
    source_files = source_files + Glob('src/win32/*.cpp')
else:
    source_files = source_files + Glob('src/posix/*.cpp')

project = ProgramProject(name, major, minor, libraries, source_files = source_files)

build = env.FreelanProject(project)
install = env.FreelanProjectInstall(project)
indent = env.FreelanProjectIndent(project)

targets = {
    'build': build,
    'install': install,
    'indent': indent,
}

Return('targets')
