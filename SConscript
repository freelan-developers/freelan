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
    'man': Dir('man'),
}

env.Append(CPPPATH=[dirs['include']])
env.Append(LIBPATH=[dirs['lib']])

env.GenerateDefines(target=os.path.join('include', env.defines.defines_file_name), source=env.defines.template_file_path)

libraries = []
includes = []

for x in Glob('libs/*'):
    sconscript_path = x.File('SConscript')

    if sconscript_path.exists():
        name = os.path.basename(str(x))

        if not sys.platform.startswith('linux'):
            if name in 'netlinkplus':
                continue

        if env.mongoose == 'no' and name in 'mongooseplus':
                continue

        if env.upnp == 'no' and name in 'miniupnpcplus':
                continue

        library, library_includes = SConscript(sconscript_path, exports='env dirs name')
        libraries.extend(library)
        includes.extend(library_includes)

apps = []
configurations = []
help = []

for x in Glob('apps/*'):
    sconscript_path = x.File('SConscript')

    if sconscript_path.exists():
        name = os.path.basename(str(x))
        app, configuration, help = SConscript(sconscript_path, exports='env dirs name')
        apps.extend(app)
        configurations.extend(configuration)

samples = []

if env.mode != 'retail':
    for x in Glob('samples/*'):
        libname = os.path.basename(str(x))

        if not sys.platform.startswith('linux'):
            if libname in 'netlinkplus':
                continue

        if libname in 'miniupnpcplus':
            continue

        for y in x.glob('*'):
            sconscript_path = y.File('SConscript')

            if sconscript_path.exists():
                name = 'sample_%s_%s' % (libname, os.path.basename(str(y)))
                sample = SConscript(sconscript_path, exports='env dirs name')
                samples.extend(sample)

                if env.mode == 'release':
                    samples.extend(env.SymLink(y.File(os.path.basename(str(y))).srcnode(), sample))
                else:
                    samples.extend(env.SymLink(y.File('%sd' % os.path.basename(str(y))).srcnode(), sample))

Return('libraries includes apps samples configurations help')
