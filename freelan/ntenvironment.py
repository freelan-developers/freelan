"""A NT based system specialized environment helper class."""

from base_environment import BaseEnvironmentHelper

import os
import platform

class EnvironmentHelper(BaseEnvironmentHelper):
    """A environment class."""

    def __init__(self, scons_module, arguments):
        """Create a new EnvironmentHelper instance."""

        super(EnvironmentHelper, self).__init__(scons_module, arguments)

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

            if self.get_architecture() != self.get_architecture(platform.machine()):
                if self.get_architecture() == '32':
                    self.environment['CXXFLAGS'].append('-m32')
                elif self.get_architecture() == '64':
                    self.environment['CXXFLAGS'].append('-m64')
        else:
            pass

    def build_library(self, name, major, minor, include_path, source_files, target_dir):
        """Build a library."""

        result = []

        result += self.environment.SharedLibrary(os.path.join(target_dir, name), source_files, CPPPATH = [include_path])

        return result
