"""The SConstruct file"""

import os

from freelan.build_tools import Environment

env = Environment(ENV=os.environ.copy(), ARGUMENTS=ARGUMENTS)

targets = SConscript('SConscript', variant_dir=env.get_variant_dir(Dir('.')), exports='env')

for name, target in targets.items():
    env.Alias(name, target)

env.Default(targets['build'])
