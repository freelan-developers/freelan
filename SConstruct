##
# libfreelan build file.
#
module = 'iconvplus'
major = '1'
minor = '0'

### YOU SHOULD NEVER CHANGE ANYTHING BELOW THIS LINE ###

import sys, os

include_path = os.path.join('include', module)
submodules = [''] + os.walk(include_path).next()[1]
source = Glob('src/*.cpp')
include = dict()
for submodule in submodules: include[submodule] = Glob(os.path.join(include_path, submodule, '*.hpp'))
cpppath = [include_path]
libs = []

# Import the customized environment
sys.path.append(os.path.abspath('scons'))
import environment

env = environment.Environment(ENV = os.environ.copy())

# Build the libraries
libraries = env.Libraries(module, major, minor, source, CPPPATH = cpppath, LIBS = libs)
documentation = env.Documentation()
indentation = env.Indentation(source + include.values())

include_install = []

for (k, v) in include.items():
    include_install += [env.Install(os.path.join(env['install_path'], 'include', module, k), v)]
libraries_install = env.Install(os.path.join(env['install_path'], env.libdir), libraries)

install = [include_install, libraries_install]

# Call the test SConstruct file
run_tests = SConscript('tests/SConscript', exports = 'env module libraries')
samples = SConscript('samples/SConscript', exports = 'env module libraries')

# Aliases
env.Alias('build', libraries)
env.Alias('install', install)
env.Alias('doc', documentation)
env.Alias('indent', indentation)
env.Alias('tests', run_tests)
env.Alias('samples', samples)
env.Alias('all', ['build', 'samples', 'doc'])
env.Alias('release', ['indent', 'all', 'tests'])

# Help documentation
Help("""Usage:

'scons build' to build the library.
'scons install' to install the library and its include files on the system.
'scons doc' to build the documentation.
'scons tests' to build the library, the tests and then run the tests.
'scons samples' to build the library and the samples.
'scons all' to build the library, the samples and the documentation.
'scons release' to indent the code, build everything then run the tests.
'scons -c' to cleanup object and libraries files.
'scons -c install' to uninstall libraries and include files.
'scons -c doc' to cleanup documentation files.
'scons -c all' to cleanup libraries and documentation files.

If scons is called without parameters, the default target is "build".

Available options:
%s""" % env.variables_help_text)

# Default
Default('build')
