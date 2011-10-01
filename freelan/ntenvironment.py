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
            self['CXXFLAGS'].append('-Wall')
            self['CXXFLAGS'].append('-Wextra')
            self['CXXFLAGS'].append('-Werror')
            self['CXXFLAGS'].append('-pedantic')
            self['CXXFLAGS'].append('-Wshadow')
            self['CXXFLAGS'].append('-Wno-long-long')
            self['CXXFLAGS'].append('-Wno-uninitialized')

            if self.mode == 'debug':
                self['CXXFLAGS'].append('-g')
            else:
                self['CXXFLAGS'].append('-O3')

            if self.arch != platform.machine():
                if tools.is_32_bits_architecture(self.arch):
                    self['CXXFLAGS'].append('-m32')
                    self['LINKFLAGS'].append('-m32')
                elif tools.is_64_bits_architecture(self.arch):
                    self['CXXFLAGS'].append('-m64')
                    self['LINKFLAGS'].append('-m64')

            self['BOOST_SUFFIX'] = {}
            self['BOOST_SUFFIX']['release'] = os.environ.get('FREELAN_MINGW_RELEASE_BOOST_SUFFIX')
            self['BOOST_SUFFIX']['debug'] = os.environ.get('FREELAN_MINGW_DEBUG_BOOST_SUFFIX', self['BOOST_SUFFIX']['release'])

            self['ARGUMENTS'].setdefault('prefix', os.environ.get('FREELAN_MINGW_INSTALL_PREFIX', os.environ.get('FREELAN_INSTALL_PREFIX', r'C:\FreeLAN')))

        else:
            if self.mode != 'debug':
                self['CXXFLAGS'].append('/O2')

            self['CXXFLAGS'].append('/EHsc')
            self['CXXFLAGS'].append('/DBOOST_ALL_NO_LIB')

            self['BOOST_SUFFIX'] = {}
            self['BOOST_SUFFIX']['release'] = os.environ.get('FREELAN_MSVC_RELEASE_BOOST_SUFFIX')
            self['BOOST_SUFFIX']['debug'] = os.environ.get('FREELAN_MSVC_DEBUG_BOOST_SUFFIX', self['BOOST_SUFFIX']['release'])

            self['ARGUMENTS'].setdefault('prefix', os.environ.get('FREELAN_MSVC_INSTALL_PREFIX', os.environ.get('FREELAN_INSTALL_PREFIX', r'C:\FreeLAN-VC')))

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

        self.__suffix_boost_libraries(env)

        shared_library = self.SharedLibrary(os.path.join(target_dir, name), source_files, **env)

        return shared_library

    def FreelanStaticLibrary(self, target_dir, name, major, minor, source_files, **env):
        """Build a static library."""

        for key, value in env.items():
            if isinstance(value, list):
                if key in self:
                    env[key] += self[key]

        static_library = self.StaticLibrary(os.path.join(target_dir, name), source_files, **env)

        return static_library

    def FreelanProgram(self, target_dir, name, source_files, **env):
        """Build a program."""

        # We add values to existing ones
        for key, value in env.items():
            if isinstance(value, list):
                if key in self:
                    env[key] += self[key]

        self.__suffix_boost_libraries(env)

        program = self.Program(os.path.join(target_dir, name), source_files, **env)

        return program

    def __suffix_boost_libraries(self, env):
        """Suffix the boost libraries found in the specified environment."""

        if self['BOOST_SUFFIX'][self.mode]:
            if 'LIBS' in env:
                env['LIBS'] = [self.__suffix_boost_library(lib) for lib in env['LIBS']]

    def __suffix_boost_library(self, lib):
        """Suffix the specified library if it belongs to boost."""

        if lib.startswith('boost_'):
            return lib + self['BOOST_SUFFIX'][self.mode]
        else:
            return lib
