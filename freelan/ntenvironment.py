"""A NT based system specialized environment helper class."""

from base_environment import BaseEnvironmentHelper

import os

class EnvironmentHelper(BaseEnvironmentHelper):
    """A environment class."""

    def __init__(self, environment_class, arguments):
        """Create a new EnvironmentHelper instance."""

        super(EnvironmentHelper, self).__init__(environment_class, arguments)

        if not 'CXXFLAGS' in self.environment:
            self.environment['CXXFLAGS'] = []

        if self.toolset == 'mingw':
            self.environment['CXXFLAGS'].append('-Wall')
            self.environment['CXXFLAGS'].append('-Wextra')
            self.environment['CXXFLAGS'].append('-Werror')
            self.environment['CXXFLAGS'].append('-pedantic')
            self.environment['CXXFLAGS'].append('-Wshadow')
            self.environment['CXXFLAGS'].append('-Wno-long-long')
            self.environment['CXXFLAGS'].append('-Wno-uninitialized')
        else:
            pass

    def build_library(self, name, major, minor, include_path, source_files, target_dir):
        """Build a library."""

        result = []

        result += self.environment.SharedLibrary(os.path.join(target_dir, name), source_files, CPPPATH = [include_path])

        return result
