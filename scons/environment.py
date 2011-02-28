from SCons.Script.SConscript import SConsEnvironment
from SCons.Script import *

import os, sys, platform

class Environment(SConsEnvironment):

	def __init__(
			self,
			os_platform=None,
			tools=None,
			toolpath=None,
			variables=None,
			parse_flags=None,
			**kw):
		
		# Variables
		self._load_variables()

		if not variables:
			variables = self._variables
		else:
			variables += self._variables

		# Tools
		if sys.platform == 'win32':
			target_tool = "mingw"
		else:
			target_tool = "default"

		if not tools:
			tools = [target_tool, 'doxygen', 'astyle']

		if not toolpath:
			toolpath = [os.path.abspath(os.path.dirname(__file__))]

		cxxflags = ['-std=c++98', '-Wall', '-Wextra', '-Werror', '-pedantic', '-Wredundant-decls', '-O3', '-Wno-uninitialized', '-Wno-long-long', '-Wshadow']
		libs = ['crypto']
		syspath = []
		libpath = []
		shlinkflags = ['-shared']

		if ARGUMENTS.get('mode', 'release') == 'debug':
			cxxflags.append('-g')
		else:
			cxxflags.append('-DNDEBUG')

		if sys.platform != 'darwin':
			if ARGUMENTS.get('arch', 0) == '32':
				cxxflags.append('-m32')
				shlinkflags.append('-m32')
			elif ARGUMENTS.get('arch', 0) == '64':
				cxxflags.append('-m64')
				shlinkflags.append('-m64')

		if sys.platform == 'win32':
			cxxflags.append('-D_WIN32_WINNT=0x0501')
			
			syspath.append(os.path.join(self._get_variable('mingw_path'), 'include'))
			syspath.append(os.path.join(self._get_variable('boost_path'), 'include', 'boost-' + self._get_variable('boost_version')))
			syspath.append(os.path.join(self._get_variable('openssl_path'), 'include'))

			libpath.append(os.path.join(self._get_variable('boost_path'), 'lib'))
			libpath.append(os.path.join(self._get_variable('openssl_path'), 'lib'))

			# Add winsock library for MS Windows
			libs.append('gdi32')

		elif sys.platform == 'darwin':
			cxxflags.append('-arch')
			cxxflags.append('i386')
			cxxflags.append('-arch')
			cxxflags.append('x86_64')
			shlinkflags.append('-arch')
			shlinkflags.append('i386')
			shlinkflags.append('-arch')
			shlinkflags.append('x86_64')
			shlinkflags.append('-single_module')
			shlinkflags.append('-undefined')
			shlinkflags.append('dynamic_lookup')

		else:
			if sys.platform.startswith('freebsd'):
				cxxflags.remove('-pedantic')
				libpath.append(os.path.join(self._get_variable('boost_path'), 'lib'))
				syspath.append(os.path.join(self._get_variable('boost_path'), 'include'))

		for path in syspath:
			cxxflags.append('-isystem' + os.path.abspath(path))

		# Parent constructor
		SConsEnvironment.__init__(self, os_platform, tools, toolpath, variables, parse_flags, **kw)

		# Save the variables
		self._save_variables()

	def _load_variables(self):
		self._variables = Variables(['variables.cache', 'variables.py'], ARGUMENTS)
		
		if sys.platform == 'win32':
			self._variables.AddVariables(PathVariable('install_path', 'The installation path', r'C:\MinGW', PathVariable.PathIsDir))
			self._variables.AddVariables(PathVariable('mingw_path', 'The path of the MinGW installation', r'C:\MinGW', PathVariable.PathIsDir))
			self._variables.AddVariables(PathVariable('boost_path', 'The path of the Boost installation', r'C:\Boost', PathVariable.PathIsDir))
			self._variables.AddVariables(PathVariable('openssl_path', 'The path of the OpenSSL installation', r'C:\OpenSSL', PathVariable.PathIsDir))
			self._variables.Add('boost_version', 'The boost version (in format x_y)', '1_44')
			self._variables.Add('boost_lib_suffix', 'The suffix for boost libraries', 'mgw45-mt')
		else:
			self._variables.AddVariables(PathVariable('boost_path', 'The path of the Boost installation', r'/usr/include', PathVariable.PathIsDir))
			self._variables.AddVariables(PathVariable('openssl_path', 'The path of the OpenSSL installation', r'/usr/include', PathVariable.PathIsDir))
			self._variables.AddVariables(PathVariable('install_path', 'The installation path', r'/usr/local', PathVariable.PathIsDir))

	def _save_variables(self):
		self._variables.Save('variables.cache', self)

	def _get_variable(self, name):
		return ''
