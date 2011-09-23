"""A base environment helper class."""

import os
import platform

class BaseEnvironmentHelper(object):
    """A base environment class."""

    def __init__(self, scons_module, arguments):
        """Create a new BaseEnvironmentHelper instance."""

        super(BaseEnvironmentHelper, self).__init__()

        self.mode = arguments.get('mode', 'release')

        if not self.mode in ['release', 'debug']:
            raise ValueError('\"mode\" can be either \"release\" or \"debug\"')

        self.arch = arguments.get('arch', platform.machine())
        self.toolset = arguments.get('toolset', os.environ.get('FREELAN_TOOLSET', 'default'))

        self.environment = scons_module.Environment.Environment(None, [self.toolset], None, None, None, ENV = os.environ.copy())

    def get_architecture(self, arch=None):
        """Get the current architecture ('32' or '64')."""

        if arch is None:
            arch = self.arch

        if arch in ['x64', '64', 'x86_64']:
            return '64'
        elif arch in ['x86', '32', 'i386']:
            return '32'
        else:
            raise ValueError('Unknown architecture \"%s\"' % arch)
