"""
The main SConscript file.
"""

name = 'cryptoplus'
major = '2'
minor = '0'
libraries = []

# You should not need to modify anything below this line

Import('env')

env = env.Clone()

import os

from freelan.build_tools import LibraryProject

if env['CC'] == 'gcc':
    libraries.append('crypto')
    if os.name == 'nt':
        libraries.append('gdi32')
else:
    libraries.append('libeay32')

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
