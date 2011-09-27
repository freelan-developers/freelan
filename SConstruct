"""The SConstruct file"""

import os

from freelan.build_tools import LibraryProject, Environment

env = Environment(ENV = os.environ.copy(), ARGUMENTS = ARGUMENTS)
project = LibraryProject()

env.FreelanProject(project)
