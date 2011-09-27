"""A base environment class."""

import os
import platform

from SCons.Script.SConscript import SConsEnvironment

class BaseEnvironment(SConsEnvironment):
    """A base environment class."""

    def __init__(
        self,
        _platform=None,
        _tools=None,
        toolpath=None,
        variables=None,
        parse_flags=None,
        **kw
    ):
        """Create a new BaseEnvironment instance."""

        if _tools is None:
            toolset = kw.setdefault('ARGUMENTS', {}).get('toolset', os.environ.get('FREELAN_TOOLSET', 'default'))
            _tools = [toolset, 'astyle', 'doxygen']

        if toolpath is None:
            toolpath = [os.path.abspath(os.path.dirname(__file__))]

        SConsEnvironment.__init__(
            self,
            _platform,
            _tools,
            toolpath,
            variables,
            parse_flags,
            **kw
        )

        self.arch = kw.setdefault('ARGUMENTS', {}).get('arch', platform.machine())
        self.mode = kw.setdefault('ARGUMENTS', {}).get('mode', 'release')
        self.libdir = kw.setdefault('ARGUMENTS', {}).get('libdir', 'lib')

        if not self.mode in ['release', 'debug']:
            raise ValueError('\"mode\" can be either \"release\" or \"debug\"')

        if not 'CXXFLAGS' in self:
            self['CXXFLAGS'] = []

        if not 'LINKFLAGS' in self:
            self['LINKFLAGS'] = []

        if not 'SHLINKFLAGS' in self:
            self['SHLINKFLAGS'] = []

    def FreelanProject(self, project):
        """Build a FreeLAN project."""

        return project.configure_environment(self)
