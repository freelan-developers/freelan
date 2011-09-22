"""A base environment helper class."""

import platform
import os

class BaseEnvironmentHelper(object):
    """A base environment class."""

    def __init__(self, environment_class, arguments):
        """Create a new BaseEnvironmentHelper instance."""

        super(BaseEnvironmentHelper, self).__init__()

        self.environment_class = environment_class
        self.arguments = arguments

        self.os_platform = arguments.get('os_platform')
        self.toolset = arguments.get('toolset', 'default')
        
        self.environment = self.environment_class(self.os_platform, [self.toolset], None, None, None, ENV = os.environ.copy())

        
    def get_architecture(self):
        """Get the current architecture ('32' or '64')."""

        if not 'arch' in self.environment:
            arch = platform.machine()
        else:
            arch = self.environment['arch']

        if arch in ['x64', '64', 'x86_64']:
            return '64'
        else:
            return '32'
