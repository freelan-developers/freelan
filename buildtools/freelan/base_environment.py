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

        environ = kw.get('ENV', {})

        if _tools is None:
            toolset = kw.setdefault('ARGUMENTS', {}).get('toolset', environ.get('FREELAN_TOOLSET', 'default'))
            _tools = [toolset, 'astyle', 'doxygen', 'nsis', 'innosetup']

        if toolpath is None:
            toolpath = [os.path.abspath(os.path.dirname(__file__))]

        self.arch = kw.setdefault('ARGUMENTS', {}).get('arch', environ.get('FREELAN_ARCH', platform.machine()))

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

        for flag in [
            'ARFLAGS',
            'CFLAGS',
            'CXXFLAGS',
            'RANLIBFLAGS',
            'LINKFLAGS',
            'SHLINKFLAGS',
        ]:
            if flag in self.environ:
                self.Append(**{flag: self.environ[flag]})

        for flag in [
            'AR',
            'CC',
            'CXX',
            'LINK',
            'SHLINK',
            'RANLIB',
        ]:
            if flag in self.environ:
                self[flag] = self.environ[flag]

        self.mode = kw.setdefault('ARGUMENTS', {}).get('mode', self.environ.get('FREELAN_MODE', 'release'))
        self.bindir = kw.setdefault('ARGUMENTS', {}).get('bindir', self.environ.get('FREELAN_BINDIR', 'bin'))
        self.libdir = kw.setdefault('ARGUMENTS', {}).get('libdir', self.environ.get('FREELAN_LIBDIR', 'lib'))
        self.static_suffix = kw.setdefault('ARGUMENTS', {}).get('static_suffix', self.environ.get('FREELAN_STATIC_SUFFIX', '_static'))

        if not self.mode in ['release', 'debug']:
            raise ValueError('\"mode\" can be either \"release\" or \"debug\"')

        self['BOOST_PREFIX'] = {}
        self['BOOST_PREFIX']['release'] = self.environ.get('FREELAN_MINGW_RELEASE_BOOST_PREFIX')
        self['BOOST_PREFIX']['debug'] = self.environ.get('FREELAN_MINGW_DEBUG_BOOST_PREFIX', self['BOOST_PREFIX']['release'])
        self['BOOST_SUFFIX'] = {}
        self['BOOST_SUFFIX']['release'] = self.environ.get('FREELAN_MINGW_RELEASE_BOOST_SUFFIX')
        self['BOOST_SUFFIX']['debug'] = self.environ.get('FREELAN_MINGW_DEBUG_BOOST_SUFFIX', self['BOOST_SUFFIX']['release'])

    @property
    def environ(self):
        return self['ENV'] if 'ENV' in self else {}

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
