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

        library, library_includes = SConscript(sconscript_path, exports='env dirs name')
        libraries.extend(library)
        includes.extend(library_includes)

apps = []
configurations = []

for x in Glob('apps/*'):
    sconscript_path = x.File('SConscript')

    if sconscript_path.exists():
        name = os.path.basename(str(x))
        app, configuration = SConscript(sconscript_path, exports='env dirs name')
        apps.extend(app)
        configurations.extend(configuration)

samples = []

if env.mode != 'retail':
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

                if env.mode == 'release':
                    samples.extend(env.SymLink(y.File(os.path.basename(str(y))).srcnode(), sample))
                else:
                    samples.extend(env.SymLink(y.File('%sd' % os.path.basename(str(y))).srcnode(), sample))

# API
libraries = [
    'boost_system',
    'boost_thread',
    'boost_filesystem',
    'boost_date_time',
    'boost_program_options',
    'boost_iostreams',
    'curl',
    'ssl',
    'crypto',
]

if sys.platform.startswith('linux'):
    libraries.extend([
        'pthread',
    ])
elif sys.platform.startswith('darwin'):
    libraries.extend([
        'ldap',
        'z',
        'iconv',
    ])

env = env.Clone()
env.Prepend(LIBS=libraries)

api_env = env.Clone()
api_env.Append(CPPDEFINES="FREELAN_API_EXPORTS")

api_sources = api_env.RGlob('src/internal', '*.cpp')
api_sources += api_env.RGlob('src/freelan', '*.cpp')
api_includes = api_env.RGlob('include/freelan', '*.h')
api_libraries = api_env.SharedLibrary(
    target=os.path.join(str(dirs['lib']), 'freelan'),
    source=api_sources,
)

tests_env = env.Clone()
tests_env.Append(CPPPATH=[
    'extra/gtest-1.7.0/include',
    'extra/gtest-1.7.0',
])

tests_sources = tests_env.RGlob('src/tests', '*.cpp')
tests_sources.append(tests_env.File('extra/gtest-1.7.0/src/gtest-all.cc'))
tests_sources.append(tests_env.File('extra/gtest-1.7.0/src/gtest_main.cc'))
tests_binaries = tests_env.Program(
    target=os.path.join(str(dirs['bin']), 'freelan_tests'),
    source=tests_sources,
)

Return(' '.join([
    'libraries',
    'includes',
    'apps',
    'samples',
    'configurations',
    'api_includes',
    'api_libraries',
    'tests_binaries',
]))
