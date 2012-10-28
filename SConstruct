"""The SConstruct file"""

import os

from freelan.build_tools import Environment

env = Environment(ENV=os.environ.copy(), ARGUMENTS=ARGUMENTS)

libraries = [
    'libasiotap',
    'libcryptoplus',
    'libfscp',
    'libfreelan',
    'libkfather',
    'libiconvplus',
    'freelan',
]

targets_names = [
    'build',
    'install',
    'documentation',
    'indent',
    'samples',
]

targets = {}

for library in libraries:
    targets[library] = SConscript(os.path.join(library, 'SConscript'), exports='env')

    # Special alias to build the libraries
    env.Alias(library, targets[library]['build'])

    for target_name in targets_names:
        if target_name in targets[library]:
            env.Alias(library + '_' + target_name, targets[library][target_name])
            env.Alias(target_name, targets[library][target_name])

Default('freelan')
