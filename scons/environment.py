from SCons.Script.SConscript import SConsEnvironment
from SCons.Script import *

import os, sys, platform

class Environment(SConsEnvironment):

	_VARIABLE_FILE = 'variables.cache'

	def __init__(
			self,
			os_platform=None,
			parse_flags=None,
			**kw):
		
		# Members
		self._libdir = 'lib'

		# Tools
		if sys.platform == 'win32':
			target_tool = "mingw"
		else:
			target_tool = "default"

		tools = [target_tool, 'doxygen', 'astyle']
		toolpath = [os.path.abspath(os.path.dirname(__file__))]

		# Variables
		variables = Environment._create_variables()

		# Parent constructor
		SConsEnvironment.__init__(self, os_platform, tools, toolpath, variables, parse_flags, **kw)

		variables.Save(Environment._VARIABLE_FILE, self)

		if not 'CXXFLAGS' in self:
			self['CXXFLAGS'] = []

		if not 'LIBPATH' in self:
			self['LIBPATH'] = []

		if not 'LIBS' in self:
			self['LIBS'] = []

		# Build with debug symbols or not
		if ARGUMENTS.get('mode', 'release') == 'debug':
			self['CXXFLAGS'].append('-g')
		else:
			self['CXXFLAGS'].append('-DNDEBUG')

		self['CXXFLAGS'] += ['-std=c++98', '-Wall', '-Wextra', '-Werror', '-pedantic', '-Wredundant-decls', '-O3', '-Wno-uninitialized', '-Wno-long-long', '-Wshadow']

		if sys.platform != 'darwin':
			if ARGUMENTS.get('arch', platform.machine()) == 'i386':
				self['CXXFLAGS'].append('-m32')
				self['LINKFLAGS'].append('-m32')
			elif ARGUMENTS.get('arch', platform.machine()) == '64':
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

			# Add winsock library for MS Windows
			self['LIBS'].append('gdi32')
		else:
			if sys.platform.startswith('freebsd'):
				self['CXXFLAGS'].remove('-pedantic')
				self['CXXFLAGS'].append('-isystem' + os.path.abspath(os.path.join(self['boost_path'], 'include')))

				self['LIBPATH'].append(os.path.join(self['boost_path'], 'lib'))

	def Libraries(self, module, major, minor, source, **kw):

		shared_source = self.SharedObject(source, **kw)

		if sys.platform == 'win32':
			static_source = shared_source
			shlinkflags = ['-Wl,--output-def,%s' % os.path.join(self._libdir, module + '.def')]
		else:
			static_source = self.StaticObject(source, **kw)
			if sys.platform == 'darwin':
				shlinkflags = ['-arch', 'i386', '-arch', 'x86_64', '-single_module', '-undefined', 'dynamic_lookup']
			else:
				shlinkflags = ['-Wl,-soname,lib%s.so.%s' % (module, major)]

		devel_shared_library = self.SharedLibrary(os.path.join(self._libdir, module), shared_source, **kw)
		static_library = self.StaticLibrary(module + '_static', static_source, **kw)

		if sys.platform == 'win32':
			shared_library = devel_shared_library
		else:
			shared_library = self.Command(os.path.join(self._libdir, 'lib%s.so.%s.%s' % (module, major, minor)), devel_shared_library, Copy("$TARGET", "$SOURCE"))		

		return [devel_shared_library, shared_library, static_library]

	def Documentation(self, **kw):
		doxygen = self.Doxygen('doxyfile', **kw)
		AlwaysBuild(doxygen)
		return doxygen

	def Indentation(self, source, **kw):
		astyle = self.AStyle(source);
		AlwaysBuild(astyle)
		return astyle

	@staticmethod
	def _create_variables():
		variables = Variables([Environment._VARIABLE_FILE], ARGUMENTS)
		
		if sys.platform == 'win32':
			variables.AddVariables(PathVariable('install_path', 'The installation path', r'C:\MinGW', PathVariable.PathIsDir))
			variables.AddVariables(PathVariable('mingw_path', 'The path of the MinGW installation', r'C:\MinGW', PathVariable.PathIsDir))
			variables.AddVariables(PathVariable('boost_path', 'The path of the Boost installation', r'C:\Boost', PathVariable.PathIsDir))
			variables.AddVariables(PathVariable('openssl_path', 'The path of the OpenSSL installation', r'C:\OpenSSL', PathVariable.PathIsDir))
			variables.Add('boost_version', 'The boost version (in format x_y)', '1_44')
			variables.Add('boost_lib_suffix', 'The suffix for boost libraries', 'mgw45-mt')
		else:
			variables.AddVariables(PathVariable('install_path', 'The installation path', r'/usr/local', PathVariable.PathIsDir))
			variables.AddVariables(PathVariable('boost_path', 'The path of the Boost installation', r'/usr/include', PathVariable.PathIsDir))
			variables.AddVariables(PathVariable('openssl_path', 'The path of the OpenSSL installation', r'/usr/include', PathVariable.PathIsDir))

		return variables
