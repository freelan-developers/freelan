"""The SConstruct file"""

name = 'cryptoplus'
major = '1'
minor = '3'
libraries = []

# You should not need to modify anything below this line

import os

from freelan.build_tools import LibraryProject, Environment

env = Environment(ENV = os.environ.copy(), ARGUMENTS = ARGUMENTS)

if env['CC'] == 'gcc':
    libraries.append('crypto')
    if os.name == 'nt':
        libraries.append('gdi32')
else:
    libraries.append('libeay32')

project = LibraryProject(name, major, minor, libraries, Glob('src/*.cpp'))

env.FreelanProject(project)

samples = env.SConscript('samples/SConscript', exports = 'env project')
env.Alias('samples', samples)
