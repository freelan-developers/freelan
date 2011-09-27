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

        if self.arch != platform.machine():
            if tools.is_32_bits_architecture(self.arch):
                self['CXXFLAGS'].append('-m32')
                self['LINKFLAGS'].append('-m32')
            elif tools.is_64_bits_architecture(self.arch):
                self['CXXFLAGS'].append('-m64')
                self['LINKFLAGS'].append('-m64')

        self['ARGUMENTS'].setdefault('prefix', '/usr/local')
        self['ARGUMENTS'].setdefault('openssl_path', '/usr')
        self['ARGUMENTS'].setdefault('boost_path', '/usr')
        self['ARGUMENTS'].setdefault('cryptoplus_path', '/usr/local')
        self['ARGUMENTS'].setdefault('asiotap_path', '/usr/local')
        self['ARGUMENTS'].setdefault('iconvplus_path', '/usr/local')
        self['ARGUMENTS'].setdefault('fscp_path', '/usr/local')
        self['ARGUMENTS'].setdefault('freelan_path', '/usr/local')

    def FreelanLibrary(self, target_dir, name, major, minor, include_path, source_files, libraries):
        """Build a library."""

        kw = {
            'CPPPATH': [include_path],
            'SHLINKFLAGS': ['-Wl,-soname,lib%s.so.%s' % (name, major)]
        }

        for library in libraries:
            self.__update_environment_from_library(kw, library)

        for key, value in kw.items():
            if isinstance(value, list):
                if key in self:
                    kw[key] += self[key]

                kw[key][:] = tools.unique(kw[key])

        shared_library = self.SharedLibrary(os.path.join(target_dir, name), source_files, **kw)
        static_library = self.StaticLibrary(os.path.join(target_dir, name + '_static'), source_files, **kw)
        versioned_shared_library = self.Command(os.path.join(target_dir, 'lib%s.so.%s.%s' % (name, major, minor)), shared_library, SCons.Script.Copy("$TARGET", "$SOURCE"))

        return shared_library + static_library + versioned_shared_library

    def __update_environment_from_library(self, env, library):
        """Update the environment according to the specified library."""

        if library.startswith('openssl_'):
            env.setdefault('CPPPATH', []).append(os.path.join(self['ARGUMENTS']['openssl_path'], 'include'))
            env.setdefault('LIBPATH', []).append(os.path.join(self['ARGUMENTS']['openssl_path'], 'lib'))

        if library == 'openssl_ssl':
            env.setdefault('LIBS', []).append('ssl')

        if library == 'openssl_crypto':
            env.setdefault('LIBS', []).append('crypto')

        if library.startswith('boost'):
            env.setdefault('CXXFLAGS', []).append('-isystem' + os.path.join(self['ARGUMENTS']['boost_path'], 'include'))

            if library.startswith('boost_'):
                env.setdefault('LIBS', []).append(library)

        if library.startswith('cryptoplus'):
            env.setdefault('LIBS', []).append(library)
            env.setdefault('CPPPATH', []).append(os.path.join(self['ARGUMENTS']['cryptoplus_path'], 'include'))
            env.setdefault('LIBPATH', []).append(os.path.join(self['ARGUMENTS']['cryptoplus_path'], 'lib'))

        if library.startswith('asiotap'):
            env.setdefault('LIBS', []).append(library)
            env.setdefault('CPPPATH', []).append(os.path.join(self['ARGUMENTS']['asiotap_path'], 'include'))
            env.setdefault('LIBPATH', []).append(os.path.join(self['ARGUMENTS']['asiotap_path'], 'lib'))

        if library.startswith('fscp'):
            env.setdefault('LIBS', []).append(library)
            env.setdefault('CPPPATH', []).append(os.path.join(self['ARGUMENTS']['fscp_path'], 'include'))
            env.setdefault('LIBPATH', []).append(os.path.join(self['ARGUMENTS']['fscp_path'], 'lib'))

        if library.startswith('iconvplus'):
            env.setdefault('LIBS', []).append(library)
            env.setdefault('CPPPATH', []).append(os.path.join(self['ARGUMENTS']['iconvplus_path'], 'include'))
            env.setdefault('LIBPATH', []).append(os.path.join(self['ARGUMENTS']['iconvplus_path'], 'lib'))

        if library.startswith('freelan'):
            env.setdefault('LIBS', []).append(library)
            env.setdefault('CPPPATH', []).append(os.path.join(self['ARGUMENTS']['freelan_path'], 'include'))
            env.setdefault('LIBPATH', []).append(os.path.join(self['ARGUMENTS']['freelan_path'], 'lib'))

