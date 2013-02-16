"""A Mac OS X based system specialized environment class."""

from freelan.posixenvironment import PosixEnvironment

import os

class MacEnvironment(PosixEnvironment):
    """An Mac OS X environment class."""

    def __init__(
        self,
        _platform=None,
        _tools=None,
        toolpath=None,
        variables=None,
        parse_flags=None,
        **kw
    ):
        """Create a new MacEnvironment instance."""

        PosixEnvironment.__init__(
            self,
            _platform,
            _tools,
            toolpath,
            variables,
            parse_flags,
            **kw
        )

        if not 'CXXFLAGS' in self.environ:
            for flag in ['-m64', '-m32']:
                if flag in self['CXXFLAGS']:
                    self['CXXFLAGS'].remove(flag)

            self.Append(CXXFLAGS='-arch')
            self.Append(CXXFLAGS='x86_64')

        if not 'LINKFLAGS' in self.environ:
            for flag in ['-m64', '-m32']:
                if flag in self['LINKFLAGS']:
                    self['LINKFLAGS'].remove(flag)

            self.Append(LINKFLAGS='-arch')
            self.Append(LINKFLAGS='x86_64')

        if not 'LIBPATH' in self.environ:
            # If compiled from sources, additionnal libraries are in /usr/local/lib
            self.Append(LIBPATH='/usr/local/lib')

    def FreelanSharedLibrary(self, target_dir, name, major, minor, source_files, **env):
        """Build a shared library."""

        # We add values to existing ones
        for key, value in env.items():
            if isinstance(value, list):
                if key in self:
                    env[key] += self[key]

        self.FixBoostLibraries(env)

        shared_library = self.SharedLibrary(os.path.join(target_dir, name), source_files, **env)

        return shared_library

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
