"""A POSIX based system specialized environment class."""

from base_environment import BaseEnvironment

import os
import platform

import SCons

import tools

class PosixEnvironment(BaseEnvironment):
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
        """Create a new PosixEnvironment instance."""

        BaseEnvironment.__init__(
            self,
            _platform,
            _tools,
            toolpath,
            variables,
            parse_flags,
            **kw
        )

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

        self['ARGUMENTS'].setdefault('prefix', '/usr/local')

    def FreelanSharedLibrary(self, target_dir, name, major, minor, source_files, **env):
        """Build a shared library."""

        env.setdefault('SHLINKFLAGS', []).append('-Wl,-soname,lib%s.so.%s' % (name, major))

        # We add values to existing ones
        for key, value in env.items():
            if isinstance(value, list):
                if key in self:
                    env[key] += self[key]

        shared_library = self.SharedLibrary(os.path.join(target_dir, name), source_files, **env)
        versioned_shared_library = self.Command(os.path.join(target_dir, 'lib%s.so.%s.%s' % (name, major, minor)), shared_library, SCons.Script.Copy("$TARGET", "$SOURCE"))

        return shared_library + versioned_shared_library

    def FreelanStaticLibrary(self, target_dir, name, major, minor, source_files, **env):
        """Build a static library."""

        # We add values to existing ones
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

        program = self.Program(os.path.join(target_dir, name), source_files, **env)

        return program
