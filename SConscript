"""
The main SConscript file.
"""

name = 'iconvplus'
major = '1'
minor = '1'
libraries = []

# You should not need to modify anything below this line

Import('env')

env = env.Clone()

import os, sys

from freelan.buildtools import LibraryProject

if sys.platform.startswith('win32') or sys.platform.startswith('darwin'):
    libraries.append('iconv')

libraries.append('boost_system')

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
