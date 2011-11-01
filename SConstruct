"""The SConstruct file"""

cli_name = 'freelan'
service_name = 'freelan_svc'
daemon_name = 'freeland'

major = '1'
minor = '0'
libraries = []

# You should not need to modify anything below this line

import os, sys

from freelan.build_tools import ProgramProject, Environment
import freelan.file_tools as file_tools

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

common_source_files = Glob('src/common/*.cpp')
env.VariantDir('build/service', 'src')
env.VariantDir('build/daemon', 'src')

cli_project = ProgramProject(cli_name, major, minor, libraries, source_files = Glob('src/common/*.cpp') + [os.path.join('src', 'cli.cpp')])
service_project = ProgramProject(service_name, major, minor, libraries, source_files = Glob('build/service/common/*.cpp') + [os.path.join('build/service', 'service.cpp')])
daemon_project = ProgramProject(daemon_name, major, minor, libraries, source_files = Glob('build/daemon/common/*.cpp') + [os.path.join('build/daemon', 'daemon.cpp')])

# Scan for source files
source_files = []

for root, directories, files in os.walk('src'):
    source_files += [os.path.join(root, file) for file in file_tools.filter(files, ['*.h', '*.hpp', '*.c', '*.cpp'])]

env.Indent(source_files)

env.FreelanProject(cli_project)

if sys.platform.startswith('win32'):
    env.FreelanProject(service_project)
else:
    env.FreelanProject(daemon_project)
