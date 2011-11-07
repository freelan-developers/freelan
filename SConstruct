"""The SConstruct file"""

name = 'freelan'

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
libraries.append('boost_filesystem')
libraries.append('boost_date_time')
libraries.append('ssl')
libraries.append('crypto')

if sys.platform.startswith('win32'):


    if env['CC'] == 'gcc':
        libraries.append('crypto')
        env['CXXFLAGS'].append('-DBOOST_THREAD_USE_LIB')
        env['CXXFLAGS'].append('-DBOOST_USE_WINDOWS_H')
        env['CXXFLAGS'].append('-D_WIN32_WINNT=0x0501')
        env['CXXFLAGS'].append('-D_WIN32_IE=0x0500')
        env['CXXFLAGS'].append('-DUNICODE')
    else:
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

    if sys.platform.startswith('linux2'):
        libraries.append('rt')

common_source_files = Glob('src/common/*.cpp')
env.VariantDir('build/service', 'src')
env.VariantDir('build/daemon', 'src')

project = ProgramProject(name, major, minor, libraries, source_files = Glob('src/*.cpp'))

env.Indent(project.files)

env.FreelanProject(project)
