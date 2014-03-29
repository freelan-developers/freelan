"""
A SConscript file.

Builds everything.
"""

Import('env')

env = env.Clone()

dirs = {
    'root': Dir('.'),
    'include': Dir('include'),
    'lib': Dir('lib'),
    'bin': Dir('bin'),
}

env.Append(CPPPATH=[dirs['include']])
env.Append(CPPPATH=[Dir('third-party/install/include').srcnode()])
env.Append(LIBPATH=[dirs['lib']])
env.Append(LIBPATH=[Dir('third-party/install/lib').srcnode()])

libraries = []
includes = []

for x in Glob('libs/iconvplus'):
#for x in Glob('libs/*'):
    sconscript_path = x.File('SConscript')

    if sconscript_path.exists():
        library, library_includes = SConscript(sconscript_path, exports='env dirs')
        libraries.extend(library)
        includes.extend(library_includes)

apps = []

for x in []:
#for x in Glob('apps/*'):
    sconscript_path = x.File('SConscript')

    if sconscript_path.exists():
        app = SConscript(sconscript_path, exports='env')
        apps.append(library)

Return('libraries includes apps')
