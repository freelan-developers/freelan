import os
import sys


Import('env dirs name')

env = env.Clone()

local_include_dir = Dir(os.path.join('include', name))
env.Prepend(CPPPATH=[local_include_dir])

all_sources = set(env.RGlob('src', '*.cpp'))
windows_sources = set(env.RGlob(os.path.join('src', 'windows'), '*.cpp'))
linux_sources = set(env.RGlob(os.path.join('src', 'linux'), '*.cpp'))

if sys.platform.startswith('linux'):
    sources = all_sources - windows_sources
else:
    sources = all_sources - windows_sources - linux_sources

includes = env.RInstall(dirs['root'], local_include_dir, ['*.hpp'])
library = env.StaticLibrary(target=os.path.join(str(dirs['lib']), name), source=sorted(sources, key=str))

Return('library includes')
