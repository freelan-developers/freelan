"""A base environment helper class."""

import os
import platform
import SCons

class BaseEnvironmentHelper(object):
    """A base environment class."""

    def __init__(self, arguments):
        """Create a new BaseEnvironmentHelper instance."""

        super(BaseEnvironmentHelper, self).__init__()

        self.arguments = arguments
        self.mode = self.arguments.get('mode', 'release')

        if not self.mode in ['release', 'debug']:
            raise ValueError('\"mode\" can be either \"release\" or \"debug\"')

        self.arch = self.arguments.get('arch', platform.machine())
        self.toolset = self.arguments.get('toolset', os.environ.get('FREELAN_TOOLSET', 'default'))
        tools = [self.toolset, 'astyle', 'doxygen']
        toolpath = [os.path.abspath(os.path.dirname(__file__))]

        self.environment = SCons.Environment.Environment(None, tools, toolpath, None, None, ENV = os.environ.copy())

        if not 'CXXFLAGS' in self.environment:
            self.environment['CXXFLAGS'] = []

        if not 'LINKFLAGS' in self.environment:
            self.environment['LINKFLAGS'] = []

        if not 'SHLINKFLAGS' in self.environment:
            self.environment['SHLINKFLAGS'] = []

    def get_architecture(self, arch=None):
        """Get the current architecture ('32' or '64')."""

        if arch is None:
            arch = self.arch

        if arch in ['x64', '64', 'x86_64']:
            return '64'
        elif arch in ['x86', '32', 'i386', 'i486', 'i586', 'i686']:
            return '32'
        else:
            raise ValueError('Unknown architecture \"%s\"' % arch)
