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

targets = {}

for library in libraries:
    targets[library] = SConscript(os.path.join(library, 'SConscript'), exports='env')

env.Alias('freelan', targets['freelan']['install'])
env.Alias('build', reduce(lambda x, y: x + list(y), [target.get('install',[]) for target in targets.values()], []))
env.Alias('install', reduce(lambda x, y: x + list(y), [target.get('install',[]) for target in targets.values()], []))
env.Alias('documentation', reduce(lambda x, y: x + list(y), [target.get('documentation', []) for target in targets.values()], []))
env.Alias('indent', reduce(lambda x, y: x + list(y), [target.get('indent', []) for target in targets.values()], []))
env.Alias('samples', reduce(lambda x, y: x + list(y), [target.get('samples', []) for target in targets.values()], []))

Default('freelan')
