"""The SConstruct file"""

name = 'iconvplus'
major = '1'
minor = '0'
libraries = []

# You should not need to modify anything below this line

import os, sys

from freelan.build_tools import LibraryProject, Environment

env = Environment(ENV = os.environ.copy(), ARGUMENTS = ARGUMENTS)

if sys.platform.startswith('win32'):
    libraries.append('iconv')

libraries.append('boost_system')

project = LibraryProject(name, major, minor, libraries, Glob('src/*.cpp'))

env.Indent(project.files)

env.FreelanProject(project)

samples = env.SConscript('samples/SConscript', exports = 'env project')
env.Alias('samples', samples)
