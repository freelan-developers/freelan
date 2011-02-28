##
# libcryptopen build file.
#
module = 'cryptopen'
major = '1'
minor = '0'

### YOU SHOULD NEVER CHANGE ANYTHING BELOW THIS LINE ###

import sys, os

include_path = os.path.join('include', module)
submodules = ['hash']
source = Glob('src/*.cpp')
include = Glob(os.path.join(include_path, '*.hpp'))
for submodule in submodules: include += Glob(os.path.join(include_path, submodule, '*.hpp'))
cpppath = [include_path]
libs = ['crypto']

# Import the customized environment
sys.path.append(os.path.abspath('scons'))
import environment

env = environment.Environment(ENV = os.environ.copy())

# Build the libraries
libraries = env.Libraries(module, major, minor, source, CPPPATH = cpppath, LIBS = libs)
install = env.Install()
documentation = env.Documentation()
indentation = env.Indentation(source + include)

# Aliases
env.Alias('build', libraries)
env.Alias('install', install)
env.Alias('doc', documentation)
env.Alias('indent', indentation)
env.Alias('all', ['build', 'doc'])

# Help documentation
Help("""
Type: 'scons build' to build the library.
      'scons install' to install the library and its include files on the system.
      'scons doc' to build the documentation.
      'scons all' to build everything.
      'scons -c' to cleanup object and libraries files.
      'scons -c install' to uninstall libraries and include files.
      'scons -c doc' to cleanup documentation files.
      'scons -c all' to cleanup libraries and documentation files.
      \n
      If scons is called without parameters, the default target is "build".
""")

# Default
Default('build')
