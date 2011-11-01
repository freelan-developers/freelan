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

        self.arch = kw.setdefault('ARGUMENTS', {}).get('arch', os.environ.get('FREELAN_ARCH', platform.machine()))

        kw.setdefault('TARGET_ARCH', self.arch)

        SConsEnvironment.__init__(
            self,
            _platform,
            _tools,
            toolpath,
            variables,
            parse_flags,
            **kw
        )

        self.mode = kw.setdefault('ARGUMENTS', {}).get('mode', os.environ.get('FREELAN_MODE', 'release'))
        self.bindir = kw.setdefault('ARGUMENTS', {}).get('bindir', os.environ.get('FREELAN_BINDIR', 'bin'))
        self.libdir = kw.setdefault('ARGUMENTS', {}).get('libdir', os.environ.get('FREELAN_LIBDIR', 'lib'))
        self.link = kw.setdefault('ARGUMENTS', {}).get('link', os.environ.get('FREELAN_LINK', 'static'))

        if not self.mode in ['release', 'debug']:
            raise ValueError('\"mode\" can be either \"release\" or \"debug\"')

        if not self.link in ['static', 'shared']:
            raise ValueError('\"link\" can be either \"static\" or \"shared\"')

        # Parse environment overloads
        if 'ENV' in kw:
            for key, value in kw['ENV'].items():
                if key.startswith('FREELAN_ENV_'):
                    self[key[len('FREELAN_ENV_'):]] = value

        if not 'CXXFLAGS' in self:
            self['CXXFLAGS'] = []

        if not 'LINKFLAGS' in self:
            self['LINKFLAGS'] = []

        if not 'SHLINKFLAGS' in self:
            self['SHLINKFLAGS'] = []

    def FreelanProject(self, project):
        """Build a FreeLAN project."""

        return project.configure_environment(self)

    def FreelanLibrary(self, target_dir, name, major, minor, source_files, **env):
        """Build a FreeLAN library."""

        if self.link == 'static':
            return self.FreelanStaticLibrary(target_dir, name, major, minor, source_files, **env)
        elif self.link == 'shared':
            return self.FreelanSharedLibrary(target_dir, name, major, minor, source_files, **env)

    def Indent(self, files):
        """Indent source files."""

        indentation = self.AStyle(files)

        self.AlwaysBuild(indentation)
        self.Alias('indent', indentation)

        return indentation
