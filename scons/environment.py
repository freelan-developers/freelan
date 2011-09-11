from SCons.Script.SConscript import SConsEnvironment
from SCons.Script import *

import os, sys, platform

class Environment(SConsEnvironment):

    _VARIABLE_FILE = 'variables.cache'

    def __init__(
            self,
            path=None,
            os_platform=None,
            parse_flags=None,
            **kw):
        
        # Variables
        if path:
            variable_file = os.path.join(path, Environment._VARIABLE_FILE)
        else:
            variable_file = Environment._VARIABLE_FILE

        variables = Environment._create_variables(variable_file)

        # Tools
        if sys.platform == 'win32':
            toolset = 'mingw'
        else:
            toolset = 'default'

        tools = [toolset, 'doxygen', 'astyle']
        toolpath = [os.path.abspath(os.path.dirname(__file__))]

        # Parent constructor
        SConsEnvironment.__init__(self, os_platform, tools, toolpath, variables, parse_flags, **kw)

        self.variables_help_text = variables.GenerateHelpText(self)

        variables.Save(variable_file, self)

        if not 'CXXFLAGS' in self:
            self['CXXFLAGS'] = []

        if not 'LIBPATH' in self:
            self['LIBPATH'] = []

        if not 'LIBS' in self:
            self['LIBS'] = []

        if not 'SHLINKFLAGS' in self:
            self['SHLINKFLAGS'] = []

        # Build with debug symbols or not
        if self['mode'] == 'debug':
            self['CXXFLAGS'].append('-g')
        else:
            self['CXXFLAGS'].append('-DNDEBUG')

        self['CXXFLAGS'] += ['-std=c++98', '-Wall', '-Wextra', '-Werror', '-pedantic', '-Wredundant-decls', '-Wno-uninitialized', '-Wno-long-long', '-Wshadow']

        if sys.platform != 'win32':
            # We must remove this flag until Boost::ASIO is fixed.
            self['CXXFLAGS'].remove('-Wshadow')

        if sys.platform != 'darwin':
            if self['arch'] == 'i386':
                self['CXXFLAGS'].append('-m32')
                self['LINKFLAGS'].append('-m32')
            elif self['arch'] == '64':
                self['CXXFLAGS'].append('-m64')
                self['LINKFLAGS'].append('-m64')
        else:
            self['CXXFLAGS'] += ['-arch', 'i386', '-arch', 'x86_64']

        if sys.platform == 'win32':
            self['CXXFLAGS'].append('-D_WIN32_WINNT=0x0501')
            self['CXXFLAGS'].append('-DBOOST_THREAD_USE_LIB')
            self['CXXFLAGS'].append('-DBOOST_USE_WINDOWS_H')
            self['CXXFLAGS'].append('-DTAP_ID=\\"%s\\"' % self['tap_id'])
            
            self['CXXFLAGS'].append('-isystem' + os.path.abspath(os.path.join(self['mingw_path'], 'include')))
            self['CXXFLAGS'].append('-isystem' + os.path.abspath(os.path.join(self['boost_path'], 'include', 'boost-' + self['boost_version'])))
            self['CXXFLAGS'].append('-isystem' + os.path.abspath(os.path.join(self['openssl_path'], 'include')))

            self['LIBPATH'].append(os.path.join(self['boost_path'], 'lib'))
            self['LIBPATH'].append(os.path.join(self['openssl_path'], 'lib'))
        else:
            if sys.platform.startswith('freebsd'):
                self['CXXFLAGS'].remove('-pedantic')
                self['CXXFLAGS'].append('-isystem' + os.path.abspath(os.path.join(self['boost_path'], 'include')))

                self['LIBPATH'].append(os.path.join(self['boost_path'], 'lib'))

    def Program(self, target, source, **kw):

        if 'LIBPATH' in kw:
            kw['LIBPATH'] += self['LIBPATH']

        if not 'LIBS' in kw:
            kw['LIBS'] = []

        self.__add_libs(kw)

        return super(self.__class__, self).Program(target, source, **kw)

    def Documentation(self, **kw):
        doxygen = self.Doxygen('doxyfile', **kw)
        AlwaysBuild(doxygen)
        return doxygen

    def Indentation(self, source, **kw):
        astyle = self.AStyle(source);
        AlwaysBuild(astyle)
        return astyle

    def __add_libs(self, kw):
        if sys.platform == 'win32':
            kw['LIBS'].append('freelan_static')
            kw['LIBS'].append('iconvplus_static')
            kw['LIBS'].append('fscp_static')
            kw['LIBS'].append('asiotap_static')
            kw['LIBS'].append('cryptoplus_static')
            kw['LIBS'].append('crypto')
            kw['LIBS'].append('boost_system-%s-%s' % (self['boost_lib_suffix'], self['boost_version']))
            kw['LIBS'].append('boost_thread-%s-%s' % (self['boost_lib_suffix'], self['boost_version']))
            kw['LIBS'].append('ws2_32')
            kw['LIBS'].append('gdi32')
            kw['LIBS'].append('iphlpapi')
        else:
            kw['LIBS'].append('freelan')
            kw['LIBS'].append('iconvplus')
            kw['LIBS'].append('fscp')
            kw['LIBS'].append('asiotap')
            kw['LIBS'].append('cryptoplus')
            kw['LIBS'].append('crypto')
            if sys.platform == 'linux2':
                kw['LIBS'].append('rt')
            kw['LIBS'].append('boost_system')
            kw['LIBS'].append('boost_thread')
            kw['LIBS'].append('pthread')

    @staticmethod
    def _create_variables(variable_file):
        variables = Variables([variable_file], ARGUMENTS)

        variables.Add(EnumVariable('mode', 'The compilation mode', 'release', ['release', 'debug']))
        variables.Add('arch', 'The target architecture', platform.machine())

        if sys.platform == 'win32':
            variables.AddVariables(PathVariable('install_path', 'The installation path', r'C:\freelan', PathVariable.PathIsDir))
            variables.AddVariables(PathVariable('mingw_path', 'The path of the MinGW installation', r'C:\MinGW', PathVariable.PathIsDir))
            variables.AddVariables(PathVariable('boost_path', 'The path of the Boost installation', r'C:\Boost', PathVariable.PathIsDir))
            variables.AddVariables(PathVariable('openssl_path', 'The path of the OpenSSL installation', r'C:\OpenSSL', PathVariable.PathIsDir))
            variables.AddVariables(PathVariable('freelan_path', 'The path of the libfreelan installation', r'C:\freelan', PathVariable.PathIsDir))
            variables.AddVariables(PathVariable('iconvplus_path', 'The path of the libiconvplus installation', r'C:\iconvplus', PathVariable.PathIsDir))
            variables.AddVariables(PathVariable('fscp_path', 'The path of the libfscp installation', r'C:\fscp', PathVariable.PathIsDir))
            variables.AddVariables(PathVariable('asiotap_path', 'The path of the libasiotap installation', r'C:\asiotap', PathVariable.PathIsDir))
            variables.AddVariables(PathVariable('cryptoplus_path', 'The path of the libcryptoplus installation', r'C:\cryptoplus', PathVariable.PathIsDir))
            variables.Add('boost_version', 'The boost version (in format x_y)', '1_47')
            variables.Add('boost_lib_suffix', 'The suffix for boost libraries', 'mgw45-mt')
            variables.Add('tap_id', 'The TAP adapter identifier', 'tap0901')
        else:
            variables.AddVariables(PathVariable('install_path', 'The installation path', r'/usr/local', PathVariable.PathIsDir))
            variables.AddVariables(PathVariable('boost_path', 'The path of the Boost installation', r'/usr', PathVariable.PathIsDir))
            variables.AddVariables(PathVariable('openssl_path', 'The path of the OpenSSL installation', r'/usr', PathVariable.PathIsDir))
            variables.AddVariables(PathVariable('freelan_path', 'The path of the libfreelan installation', r'/usr', PathVariable.PathIsDir))
            variables.AddVariables(PathVariable('iconvplus_path', 'The path of the libiconvplus installation', r'/usr', PathVariable.PathIsDir))
            variables.AddVariables(PathVariable('fscp_path', 'The path of the libfscp installation', r'/usr', PathVariable.PathIsDir))
            variables.AddVariables(PathVariable('asiotap_path', 'The path of the libasiotap installation', r'/usr', PathVariable.PathIsDir))
            variables.AddVariables(PathVariable('cryptoplus_path', 'The path of the libcryptoplus installation', r'/usr', PathVariable.PathIsDir))

        return variables
