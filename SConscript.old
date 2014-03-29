"""The SConscript file"""

import os

Import('env')

env = env.Clone()

# Here are the libraries, in order of compilation
libraries = [
    'libiconvplus',
    'libkfather',
    'libasiotap',
    'libcryptoplus',
    'libfscp',
    'libfreelan',
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
    targets[library] = SConscript(os.path.join(library, 'SConscript'), variant_dir=env.get_variant_dir(Dir(library)), exports='env')

    # Special alias to build the libraries
    env.Alias(library, targets[library]['build'])

    for target_name in targets_names:
        if target_name in targets[library]:
            env.Alias(library + '_' + target_name, targets[library][target_name])
            env.Alias(target_name, targets[library][target_name])

Return('targets')
