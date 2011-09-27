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

        if self.toolset == 'mingw':
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
        else:
            self['CXXFLAGS'].append('/O2')
            self['CXXFLAGS'].append('/EHsc')

        self['ARGUMENTS'].setdefault('prefix', r'C:\mingw')
        self['ARGUMENTS'].setdefault('openssl_path', r'C:\openssl')
        self['ARGUMENTS'].setdefault('boost_path', r'C:\boost')
        self['ARGUMENTS'].setdefault('boost_version', '1_47')

        if self.toolset == 'mingw':
            self['ARGUMENTS'].setdefault('boost_suffix', 'mgw45-mt')
        else:
            self['ARGUMENTS'].setdefault('boost_suffix', 'vc100-mt')

        self['ARGUMENTS'].setdefault('cryptoplus_path', r'C:\mingw')
        self['ARGUMENTS'].setdefault('asiotap_path', r'C:\mingw')
        self['ARGUMENTS'].setdefault('iconvplus_path', r'C:\mingw')
        self['ARGUMENTS'].setdefault('fscp_path', r'C:\mingw')
        self['ARGUMENTS'].setdefault('freelan_path', r'C:\mingw')

    def FreelanLibrary(self, target_dir, name, major, minor, include_path, source_files, libraries):
        """Build a library."""

        kw = {
            'CPPPATH': [include_path]
        }

        for library in libraries:
            self.__update_environment_from_library(kw, library)

        for key, value in kw.items():
            if isinstance(value, list):
                if key in self:
                    kw[key] += self[key]

                kw[key][:] = tools.unique(kw[key])

        objects = self.Object(source = source_files, **kw)
        shared_library = self.SharedLibrary(os.path.join(target_dir, name), objects, **kw)
        static_library = self.StaticLibrary(os.path.join(target_dir, name + '_static'), objects, **kw)

        return shared_library + static_library

    def __update_environment_from_library(self, env, library):
        """Update the environment according to the specified library."""

        if library.startswith('openssl_'):
            env.setdefault('CPPPATH', []).append(os.path.join(self['ARGUMENTS']['openssl_path'], 'include'))
            env.setdefault('LIBPATH', []).append(os.path.join(self['ARGUMENTS']['openssl_path'], 'lib'))

        if library == 'openssl_ssl':
            if self.toolset == 'mingw':
                env.setdefault('LIBS', []).append('ssl')
            else:
                env.setdefault('LIBS', []).append('ssleay32')

        if library == 'openssl_crypto':
            if self.toolset == 'mingw':
                env.setdefault('LIBS', []).append('crypto')
            else:
                env.setdefault('LIBS', []).append('libeay32')
            env.setdefault('LIBS', []).append('gdi32')

        if library.startswith('boost'):
            if self.toolset == 'mingw':
                env.setdefault('CXXFLAGS', []).append('-isystem' + os.path.join(self['ARGUMENTS']['boost_path'], 'include', 'boost-' + self['ARGUMENTS']['boost_version']))
            else:
                env.setdefault('CXXFLAGS', []).append('-DBOOST_ALL_NO_LIB=1')
                env.setdefault('CPPPATH', []).append(os.path.join(self['ARGUMENTS']['boost_path'], 'include', 'boost-' + self['ARGUMENTS']['boost_version']))

            if library.startswith('boost_'):
                env.setdefault('LIBPATH', []).append(os.path.join(self['ARGUMENTS']['boost_path'], 'lib'))
                env.setdefault('LIBS', []).append(library + '-%s-%s' % (self['ARGUMENTS']['boost_suffix'], self['ARGUMENTS']['boost_version']))

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

