"""A Mac OS X based system specialized environment class."""

from posixenvironment import PosixEnvironment

import os
import platform

import SCons

import tools

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

        if any("-m64" in s for s in self['CXXFLAGS']):
            self['CXXFLAGS'].remove('-m64')
            self['LINKFLAGS'].remove('-m64')
        if any("-m32" in s for s in self['CXXFLAGS']):
            self['CXXFLAGS'].remove('-m32')
            self['LINKFLAGS'].remove('-m32')

        self['CXXFLAGS'].append('-arch')
        self['CXXFLAGS'].append('x86_64')

        # if compiled from sources, additionnal libs are in /usr/local/lib       
        self['LINKFLAGS'].append('-L/usr/local/lib')
        self['LINKFLAGS'].append('-arch')
        self['LINKFLAGS'].append('x86_64')

        self['BOOST_PREFIX'] = {}
        self['BOOST_PREFIX']['release'] = os.environ.get('FREELAN_RELEASE_BOOST_PREFIX')
        self['BOOST_PREFIX']['debug'] = os.environ.get('FREELAN_DEBUG_BOOST_PREFIX', self['BOOST_PREFIX']['release'])
        self['BOOST_SUFFIX'] = {}
        self['BOOST_SUFFIX']['release'] = os.environ.get('FREELAN_RELEASE_BOOST_SUFFIX')
        self['BOOST_SUFFIX']['debug'] = os.environ.get('FREELAN_DEBUG_BOOST_SUFFIX', self['BOOST_SUFFIX']['release'])

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
