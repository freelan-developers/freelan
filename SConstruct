##
# freelan build file.
#

program_name = 'freelan'

### YOU SHOULD NEVER CHANGE ANYTHING BELOW THIS LINE ###

import sys, os

source = Glob('src/*.cpp')

# Import the customized environment
sys.path.append(os.path.abspath('scons'))
import environment

env = environment.Environment(ENV = os.environ.copy())

# Build the program
program = env.Program(os.path.join('bin', program_name), source)
documentation = env.Documentation()
indentation = env.Indentation(source)

program_install = env.Install(os.path.join(env['install_path'], 'bin'), program)

install = [program_install]

# Aliases
env.Alias('build', program)
env.Alias('install', install)
env.Alias('doc', documentation)
env.Alias('indent', indentation)
env.Alias('all', ['build', 'doc'])
env.Alias('release', ['indent', 'all'])

# Help documentation
Help("""Usage:

'scons build' to build the program.
'scons install' to install the program on the system.
'scons doc' to build the documentation.
'scons all' to build the program and the documentation.
'scons release' to indent the code and build everything.
'scons -c' to cleanup object and program files.
'scons -c install' to uninstall the program.
'scons -c doc' to cleanup documentation files.
'scons -c all' to cleanup program and documentation files.

If scons is called without parameters, the default target is "build".

Available options:
%s""" % env.variables_help_text)

# Default
Default('build')
