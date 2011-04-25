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
		
		# Members
		self._libdir = 'lib'

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

	def Libraries(self, module, major, minor, source, **kw):

		shared_source = self.SharedObject(source, **kw)

		shlinkflags = self['SHLINKFLAGS']

		if sys.platform == 'win32':
			static_source = shared_source
			shlinkflags += ['-Wl,--output-def,%s' % os.path.abspath(os.path.join(sys.path[0], self._libdir, module + '.def'))]

			if not 'LIBS' in kw:
				kw['LIBS'] = []
		else:
			static_source = self.StaticObject(source, **kw)
			if sys.platform == 'darwin':
				shlinkflags += ['-arch', 'i386', '-arch', 'x86_64', '-single_module', '-undefined', 'dynamic_lookup']
			else:
				shlinkflags += ['-Wl,-soname,lib%s.so.%s' % (module, major)]

		self.__add_libs(kw)

		devel_shared_library = self.SharedLibrary(os.path.join(self._libdir, module), shared_source, SHLINKFLAGS = shlinkflags, **kw)
		static_library = self.StaticLibrary(os.path.join(self._libdir, module + '_static'), static_source, **kw)

		if sys.platform == 'win32':
			shared_library = devel_shared_library
		else:
			shared_library = self.Command(os.path.join(self._libdir, 'lib%s.so.%s.%s' % (module, major, minor)), devel_shared_library, Copy("$TARGET", "$SOURCE"))		

		return (devel_shared_library, shared_library, static_library)

	def Sample(self, sample, source, **kw):

		if 'LIBPATH' in kw:
			kw['LIBPATH'] += self['LIBPATH']

		if not 'LIBS' in kw:
			kw['LIBS'] = []

		self.__add_libs(kw)

		return self.Program(sample, source, **kw)

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
			kw['LIBS'].append('cryptoplus_static')
			kw['LIBS'].append('crypto')
			kw['LIBS'].append('boost_system-%s-%s' % (self['boost_lib_suffix'], self['boost_version']))
			kw['LIBS'].append('ws2_32')
			kw['LIBS'].append('gdi32')
		else:
			kw['LIBS'].append('boost_system')
			kw['LIBS'].append('pthread')
			kw['LIBS'].append('cryptoplus')
			kw['LIBS'].append('crypto')

	@staticmethod
	def _create_variables(variable_file):
		variables = Variables([variable_file], ARGUMENTS)
		
		variables.Add(EnumVariable('mode', 'The compilation mode', 'release', ['release', 'debug']))
		variables.Add('arch', 'The target architecture', platform.machine())

		if sys.platform == 'win32':
			variables.AddVariables(PathVariable('install_path', 'The installation path', r'C:\MinGW', PathVariable.PathIsDir))
			variables.AddVariables(PathVariable('mingw_path', 'The path of the MinGW installation', r'C:\MinGW', PathVariable.PathIsDir))
			variables.AddVariables(PathVariable('boost_path', 'The path of the Boost installation', r'C:\Boost', PathVariable.PathIsDir))
			variables.AddVariables(PathVariable('openssl_path', 'The path of the OpenSSL installation', r'C:\OpenSSL', PathVariable.PathIsDir))
			variables.AddVariables(PathVariable('cryptoplus_path', 'The path of the libcryptoplus installation', r'C:\cryptoplus', PathVariable.PathIsDir))
			variables.Add('boost_version', 'The boost version (in format x_y)', '1_44')
			variables.Add('boost_lib_suffix', 'The suffix for boost libraries', 'mgw45-mt')
		else:
			variables.AddVariables(PathVariable('install_path', 'The installation path', r'/usr/local', PathVariable.PathIsDir))
			variables.AddVariables(PathVariable('boost_path', 'The path of the Boost installation', r'/usr/include', PathVariable.PathIsDir))
			variables.AddVariables(PathVariable('openssl_path', 'The path of the OpenSSL installation', r'/usr/include', PathVariable.PathIsDir))
			variables.AddVariables(PathVariable('cryptoplus_path', 'The path of the libcryptoplus installation', r'/usr/include', PathVariable.PathIsDir))

		return variables
