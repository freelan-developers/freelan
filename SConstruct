"""The SConstruct file"""

name = 'cryptoplus'
major = '1'
minor = '3'
libraries = []

# Libraries logic

import sys

if sys.platform == 'linux2':
    libraries.append('crypto')

# You should not need to modify anything below this line

import os
from freelan.build_tools import LibraryProject, Environment

env = Environment(ENV = os.environ.copy(), ARGUMENTS = ARGUMENTS)

project = LibraryProject(name, major, minor, libraries)

env.FreelanProject(project)

samples = env.SConscript('samples/SConscript', exports = 'env project')
env.Alias('samples', samples)
