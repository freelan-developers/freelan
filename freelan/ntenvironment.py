"""A NT based system specialized environment class."""

from base_environment import BaseEnvironment

import os
import platform

import tools

class NtEnvironment(BaseEnvironment):
    """An environment class."""

    def __init__(
        self,
        _platform=None,
        _tools=None,
        toolpath=None,
        variables=None,
        parse_flags=None,
        **kw
    ):
        """Create a new NtEnvironment instance."""

        BaseEnvironment.__init__(
            self,
            _platform,
            _tools,
            toolpath,
            variables,
            parse_flags,
            **kw
        )

        if self['CC'] == 'gcc':
            if not 'CXXFLAGS' in self.environ:
                self.Append(CXXFLAGS='-Wall')
                self.Append(CXXFLAGS='-Wextra')
                self.Append(CXXFLAGS='-Werror')
                self.Append(CXXFLAGS='-pedantic')
                self.Append(CXXFLAGS='-Wshadow')
                self.Append(CXXFLAGS='-Wno-long-long')
                self.Append(CXXFLAGS='-Wno-uninitialized')

                if self.mode == 'debug':
                    self.Append(CXXFLAGS='-g')
                else:
                    self.Append(CXXFLAGS='-O3')

                if tools.is_32_bits_architecture(self.arch):
                    self.Append(CXXFLAGS='-m32')
                elif tools.is_64_bits_architecture(self.arch):
                    self.Append(CXXFLAGS='-m64')

            if not 'LINKFLAGS' in self.environ:

                if tools.is_32_bits_architecture(self.arch):
                    self.Append(LINKFLAGS='-m32')
                elif tools.is_64_bits_architecture(self.arch):
                    self.Append(LINKFLAGS='-m64')

            self['BOOST_PREFIX'] = {}
            self['BOOST_PREFIX']['release'] = self.environ.get('FREELAN_MINGW_RELEASE_BOOST_PREFIX', self.environ.get('FREELAN_RELEASE_BOOST_PREFIX'))
            self['BOOST_PREFIX']['debug'] = self.environ.get('FREELAN_MINGW_DEBUG_BOOST_PREFIX', self.environ.get('FREELAN_DEBUG_BOOST_PREFIX', self['BOOST_PREFIX']['release']))
            self['BOOST_SUFFIX'] = {}
            self['BOOST_SUFFIX']['release'] = self.environ.get('FREELAN_MINGW_RELEASE_BOOST_SUFFIX', self.environ.get('FREELAN_RELEASE_BOOST_SUFFIX'))
            self['BOOST_SUFFIX']['debug'] = self.environ.get('FREELAN_MINGW_DEBUG_BOOST_SUFFIX', self.environ.get('FREELAN_DEBUG_BOOST_SUFFIX', self['BOOST_SUFFIX']['release']))

            self['ARGUMENTS'].setdefault('build-prefix', self.environ.get('FREELAN_MINGW_BUILD_PREFIX', self.environ.get('FREELAN_BUILD_PREFIX', None)))
            self['ARGUMENTS'].setdefault('prefix', self.environ.get('FREELAN_MINGW_INSTALL_PREFIX', self.environ.get('FREELAN_INSTALL_PREFIX', r'C:\FreeLAN')))

        else:
            if not 'CXXFLAGS' in self.environ:

                if self.mode != 'debug':
                    self.Append(CXXFLAGS='/O2')

                self.Append(CXXFLAGS='/MD')
                self.Append(CXXFLAGS='/EHsc')
                self.Append(CXXFLAGS='/DBOOST_ALL_NO_LIB')

            self['BOOST_PREFIX'] = {}
            self['BOOST_PREFIX']['release'] = self.environ.get('FREELAN_MSVC_RELEASE_BOOST_PREFIX', self.environ.get('FREELAN_RELEASE_BOOST_PREFIX'))
            self['BOOST_PREFIX']['debug'] = self.environ.get('FREELAN_MSVC_DEBUG_BOOST_PREFIX', self.environ.get('FREELAN_DEBUG_BOOST_PREFIX', self['BOOST_PREFIX']['release']))
            self['BOOST_SUFFIX'] = {}
            self['BOOST_SUFFIX']['release'] = self.environ.get('FREELAN_MSVC_RELEASE_BOOST_SUFFIX', self.environ.get('FREELAN_RELEASE_BOOST_SUFFIX'))
            self['BOOST_SUFFIX']['debug'] = self.environ.get('FREELAN_MSVC_DEBUG_BOOST_SUFFIX', self.environ.get('FREELAN_DEBUG_BOOST_SUFFIX', self['BOOST_SUFFIX']['release']))

            self['ARGUMENTS'].setdefault('build-prefix', self.environ.get('FREELAN_MSVC_BUILD_PREFIX', self.environ.get('FREELAN_BUILD_PREFIX', None)))
            self['ARGUMENTS'].setdefault('prefix', self.environ.get('FREELAN_MSVC_INSTALL_PREFIX', self.environ.get('FREELAN_INSTALL_PREFIX', r'C:\FreeLAN-VC')))

        self.Append(CPPPATH=[os.path.join(self['ARGUMENTS']['prefix'], 'include')])
        self.Append(LIBPATH=[os.path.join(self['ARGUMENTS']['prefix'], 'lib')])

    def FreelanSharedLibrary(self, target_dir, name, major, minor, source_files, **env):
        """Build a shared library."""

        if self['CC'] == 'gcc':
            env.setdefault('SHOBJSUFFIX', '.os');
        else:
            env.setdefault('SHOBJSUFFIX', '.sobj');

        for key, value in env.items():
            if isinstance(value, list):
                if key in self:
                    env[key] += self[key]

        self.FixBoostLibraries(env)

        shared_library = self.SharedLibrary(os.path.join(target_dir, name), source_files, **env)

        return shared_library

    def FreelanStaticLibrary(self, target_dir, name, major, minor, source_files, **env):
        """Build a static library."""

        for key, value in env.items():
            if isinstance(value, list):
                if key in self:
                    env[key] += self[key]

        static_library = self.StaticLibrary(os.path.join(target_dir, name + self.static_suffix), source_files, **env)

        return static_library

    def FreelanProgram(self, target_dir, name, source_files, **env):
        """Build a program."""

        # We add values to existing ones
        for key, value in env.items():
            if isinstance(value, list):
                if key in self:
                    env[key] += self[key]

        self.FixBoostLibraries(env)

        program = self.Program(os.path.join(target_dir, name), source_files, **env)

        return program
