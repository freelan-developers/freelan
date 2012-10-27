"""The SConstruct file"""

import os

from freelan.build_tools import Environment

env = Environment(ENV=os.environ.copy(), ARGUMENTS=ARGUMENTS)

targets = SConscript('SConscript', exports='env')

for name, target in targets.items():
    env.Alias(name, target)

env.Default(targets['build'])
