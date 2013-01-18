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
            _tools = [toolset, 'astyle', 'doxygen', 'nsis', 'innosetup']

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
        self.static_suffix = kw.setdefault('ARGUMENTS', {}).get('static_suffix', os.environ.get('FREELAN_STATIC_SUFFIX', '_static'))

        if not self.mode in ['release', 'debug']:
            raise ValueError('\"mode\" can be either \"release\" or \"debug\"')

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

    def get_variant_dir(self, suffix_dir=None):
        """Get the variant dir."""

        build_prefix = self['ARGUMENTS'].get('build-prefix')

        if build_prefix and suffix_dir:
            build_prefix = os.path.join(build_prefix, os.path.basename(suffix_dir.srcnode().abspath))

        return build_prefix

    def FreelanProject(self, project):
        """Build a FreeLAN project."""

        return project.configure_environment(self)

    def FreelanProjectInstall(self, project):
        """Install a FreeLAN project."""

        return project.configure_install_environment(self)

    def FreelanProjectDocumentation(self, project):
        """Generate a Freelan project's documentation."""

        return project.configure_documentation_environment(self)

    def FreelanProjectIndent(self, project):
        """Indent a project source files."""

        return self.Indent(project.files)

    def Indent(self, files):
        """Indent source files."""

        indentation = self.AStyle(files)

        self.AlwaysBuild(indentation)

        return indentation

    def FixBoostLibraries(self, env):
        """Suffix the boost libraries found in the specified environment."""

        if self['BOOST_SUFFIX'][self.mode]:
            if 'LIBS' in env:
                env['LIBS'] = [self.FixBoostLibrary(lib) for lib in env['LIBS']]

    def FixBoostLibrary(self, lib):
        """Suffix the specified library if it belongs to boost."""

        if str(lib).startswith('boost_'):
            return '%s%s%s' % (self['BOOST_PREFIX'][self.mode] or '', lib, self['BOOST_SUFFIX'][self.mode] or '')
        else:
            return lib
