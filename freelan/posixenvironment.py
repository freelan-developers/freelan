"""A POSIX based system specialized environment helper class."""

from base_environment import BaseEnvironmentHelper

import os
import platform

import SCons

import tools

class EnvironmentHelper(BaseEnvironmentHelper):
    """A environment class."""

    def __init__(self, arguments):
        """Create a new EnvironmentHelper instance."""

        super(EnvironmentHelper, self).__init__(arguments)

        self.environment['CXXFLAGS'].append('-Wall')
        self.environment['CXXFLAGS'].append('-Wextra')
        self.environment['CXXFLAGS'].append('-Werror')
        self.environment['CXXFLAGS'].append('-pedantic')
        self.environment['CXXFLAGS'].append('-Wshadow')
        self.environment['CXXFLAGS'].append('-Wno-long-long')
        self.environment['CXXFLAGS'].append('-Wno-uninitialized')

        if self.get_architecture() != self.get_architecture(platform.machine()):
            if self.get_architecture() == '32':
                self.environment['CXXFLAGS'].append('-m32')
                self.environment['LINKFLAGS'].append('-m32')
            elif self.get_architecture() == '64':
                self.environment['CXXFLAGS'].append('-m64')
                self.environment['LINKFLAGS'].append('-m64')

        self.arguments.setdefault('prefix', '/usr/local')
        self.arguments.setdefault('openssl_path', '/usr')
        self.arguments.setdefault('boost_path', '/usr')
        self.arguments.setdefault('cryptoplus_path', '/usr/local')
        self.arguments.setdefault('asiotap_path', '/usr/local')
        self.arguments.setdefault('iconvplus_path', '/usr/local')
        self.arguments.setdefault('fscp_path', '/usr/local')
        self.arguments.setdefault('freelan_path', '/usr/local')

    def build_library(self, target_dir, name, major, minor, include_path, source_files, libraries):
        """Build a library."""

        environment = {
            'CPPPATH': [include_path],
            'SHLINKFLAGS': ['-Wl,-soname,lib%s.so.%s' % (name, major)]
        }

        for library in libraries:
            self.update_environment_from_library(environment, library)

        for key, value in environment.items():
            if isinstance(value, list):
                if key in self.environment:
                    environment[key] += self.environment[key]

                environment[key][:] = tools.unique(environment[key])

        shared_library = self.environment.SharedLibrary(os.path.join(target_dir, name), source_files, **environment)
        static_library = self.environment.StaticLibrary(os.path.join(target_dir, name + '_static'), source_files, **environment)
        versioned_shared_library = self.environment.Command(os.path.join(target_dir, 'lib%s.so.%s.%s' % (name, major, minor)), shared_library, SCons.Script.Copy("$TARGET", "$SOURCE"))

        return shared_library, static_library + versioned_shared_library

    def update_environment_from_library(self, env, library):
        """Update the environment according to the specified library."""

        if library.startswith('openssl_'):
            env.setdefault('CPPPATH', []).append(os.path.join(self.arguments['openssl_path'], 'include'))
            env.setdefault('LIBPATH', []).append(os.path.join(self.arguments['openssl_path'], 'lib'))

        if library == 'openssl_ssl':
            env.setdefault('LIBS', []).append('ssl')

        if library == 'openssl_crypto':
            env.setdefault('LIBS', []).append('crypto')

        if library.startswith('boost'):
            env.setdefault('CXXFLAGS', []).append('-isystem' + os.path.join(self.arguments['boost_path'], 'include'))

            if library.startswith('boost_'):
                env.setdefault('LIBS', []).append(library)

        if library.startswith('cryptoplus'):
            env.setdefault('LIBS', []).append(library)
            env.setdefault('CPPPATH', []).append(os.path.join(self.arguments['cryptoplus_path'], 'include'))
            env.setdefault('LIBPATH', []).append(os.path.join(self.arguments['cryptoplus_path'], 'lib'))

        if library.startswith('asiotap'):
            env.setdefault('LIBS', []).append(library)
            env.setdefault('CPPPATH', []).append(os.path.join(self.arguments['asiotap_path'], 'include'))
            env.setdefault('LIBPATH', []).append(os.path.join(self.arguments['asiotap_path'], 'lib'))

        if library.startswith('fscp'):
            env.setdefault('LIBS', []).append(library)
            env.setdefault('CPPPATH', []).append(os.path.join(self.arguments['fscp_path'], 'include'))
            env.setdefault('LIBPATH', []).append(os.path.join(self.arguments['fscp_path'], 'lib'))

        if library.startswith('iconvplus'):
            env.setdefault('LIBS', []).append(library)
            env.setdefault('CPPPATH', []).append(os.path.join(self.arguments['iconvplus_path'], 'include'))
            env.setdefault('LIBPATH', []).append(os.path.join(self.arguments['iconvplus_path'], 'lib'))

        if library.startswith('freelan'):
            env.setdefault('LIBS', []).append(library)
            env.setdefault('CPPPATH', []).append(os.path.join(self.arguments['freelan_path'], 'include'))
            env.setdefault('LIBPATH', []).append(os.path.join(self.arguments['freelan_path'], 'lib'))

