"""
A SConscript file.

Builds everything.
"""

import os
import sys


Import('env')

env = env.Clone()

dirs = {
    'root': Dir('.'),
    'include': Dir('include'),
    'lib': Dir('lib'),
    'bin': Dir('bin'),
}

env.Append(CPPPATH=[dirs['include']])
env.Append(CXXFLAGS=['-isystem', Dir('third-party/install/include').srcnode()])
env.Append(LIBPATH=[dirs['lib']])
env.Append(LIBPATH=[Dir('third-party/install/lib').srcnode()])

libraries = []
includes = []

for x in Glob('libs/*'):
    sconscript_path = x.File('SConscript')

    if sconscript_path.exists():
        name = os.path.basename(str(x))

        if not sys.platform.startswith('linux'):
            if name in 'netlinkplus':
                continue

        library, library_includes = SConscript(sconscript_path, exports='env dirs name')
        libraries.extend(library)
        includes.extend(library_includes)

apps = []

for x in Glob('apps/*'):
    sconscript_path = x.File('SConscript')

    if sconscript_path.exists():
        name = os.path.basename(str(x))
        apps.extend(SConscript(sconscript_path, exports='env dirs name'))

samples = []

for x in Glob('samples/*'):
    libname = os.path.basename(str(x))

    if not sys.platform.startswith('linux'):
        if libname in 'netlinkplus':
            continue

    for y in x.glob('*'):
        sconscript_path = y.File('SConscript')

        if sconscript_path.exists():
            name = 'sample_%s_%s' % (libname, os.path.basename(str(y)))
            sample = SConscript(sconscript_path, exports='env dirs name')
            samples.extend(sample)

            if env.debug:
                samples.extend(env.SymLink(y.File('%sd' % os.path.basename(str(y))).srcnode(), sample))
            else:
                samples.extend(env.SymLink(y.File(os.path.basename(str(y))).srcnode(), sample))

Return('libraries includes apps samples')
