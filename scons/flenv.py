import sys, platform, os
from SCons.Script import *

def BoostLibraries(short_names):
	result = []
	for short_name in short_names:
		if sys.platform == 'win32':
			result.append('boost_' + short_name + '-' + varenv['boost_lib_suffix'] + '-' + varenv['boost_version'])
		else:
			result.append('boost_' + short_name)

	return result
# Options
_variables = Variables(['variables.cache', 'variables.py'], ARGUMENTS)

_variables.Add('without_libxmlsec', 'Whether to use libxmlsec or not', '0')

if sys.platform == 'win32':
	_variables.AddVariables(PathVariable('install_path', 'The installation path', r'C:\MinGW', PathVariable.PathIsDir))
	_variables.AddVariables(PathVariable('mingw_path', 'The path of the MinGW installation', r'C:\MinGW', PathVariable.PathIsDir))
	_variables.AddVariables(PathVariable('boost_path', 'The path of the Boost installation', r'C:\Boost', PathVariable.PathIsDir))
	_variables.AddVariables(PathVariable('openssl_path', 'The path of the OpenSSL installation', r'C:\OpenSSL', PathVariable.PathIsDir))
	_variables.AddVariables(PathVariable('libxml2_path', 'The path of the libxml2 installation', r'C:\libxml2', PathVariable.PathIsDir))
	_variables.AddVariables(PathVariable('libxmlsec_path', 'The path of the libxmlsec installation', r'C:\libxmlsec', PathVariable.PathIsDir))
	_variables.Add('boost_version', 'The boost version (in format x_y)', '1_44')
	_variables.Add('boost_lib_suffix', 'The suffix for boost libraries', 'mgw45-mt')
else:
	_variables.AddVariables(PathVariable('install_path', 'The installation path', r'/usr/local', PathVariable.PathIsDir))

	if sys.platform == 'darwin':
		_variables.AddVariables(PathVariable('libxml2_path', 'The path of the libxml2 installation', r'/usr/local', PathVariable.PathIsDir))
		_variables.AddVariables(PathVariable('libxmlsec_path', 'The path of the libxmlsec installation', r'/usr/local', PathVariable.PathIsDir))
	elif sys.platform.startswith('freebsd'):
		_variables.AddVariables(PathVariable('boost_path', 'The path of the Boost installation', r'/usr/local', PathVariable.PathIsDir))
		_variables.AddVariables(PathVariable('libxml2_path', 'The path of the libxml2 installation', r'/usr/local', PathVariable.PathIsDir))
		_variables.AddVariables(PathVariable('libxmlsec_path', 'The path of the libxmlsec installation', r'/usr/local', PathVariable.PathIsDir))
	else:
		_variables.AddVariables(PathVariable('libxml2_path', 'The path of the libxml2 installation', r'/usr', PathVariable.PathIsDir))
		_variables.AddVariables(PathVariable('libxmlsec_path', 'The path of the libxmlsec installation', r'/usr', PathVariable.PathIsDir))

varenv = Environment(variables = _variables)

# Platform
if sys.platform == 'win32':
	target_platform = "mingw"
else:
	target_platform = "default"

# Configure compiler arguments
cxxflags = ['-std=c++98', '-Wall', '-Wextra', '-Werror', '-pedantic', '-Wredundant-decls', '-O3']
libs = ['xml2', 'z', 'ssl', 'crypto']
# Beware ! These paths must be absolute so they can be used for the samples !
cpppath = []
libpath = []
syspath = []

if varenv['without_libxmlsec'] == '0':
	libs.insert(0, 'xmlsec1')
	libs.insert(0, 'xmlsec1-openssl')
	
	# xmlsec defines
	cxxflags.append('-DXMLSEC_NO_SIZE_T')
	cxxflags.append('-DXMLSEC_NO_XSLT=1')
	cxxflags.append('-DXMLSEC_CRYPTO_OPENSSL=1')
	cxxflags.append('-DXMLSEC_NO_CRYPTO_DYNAMIC_LOADING=1')
	cxxflags.append('-DXMLSEC_CRYPTO=\\"openssl\\"')

# Windows specific adjustments
if sys.platform == 'win32':
	# Remove flags that cause compilation errors
	cxxflags.remove('-std=c++98') #::swprintf and ::vswprintf are not declared
	cxxflags.append('-Wno-uninitialized')
	cxxflags.append('-Wno-long-long')
	cxxflags.append('-Wno-redundant-decls')
	cxxflags.append('-D_WIN32_WINNT=0x0501')
	cxxflags.append('-DLIBXML_STATIC')
	
	syspath.append(os.path.join(varenv['mingw_path'], 'include'))
	syspath.append(os.path.join(varenv['boost_path'], 'include', 'boost-' + varenv['boost_version']))
	syspath.append(os.path.join(varenv['openssl_path'], 'include'))
	syspath.append(os.path.join(varenv['libxml2_path'], 'include', 'libxml2'))

	libpath.append(os.path.join(varenv['libxml2_path'], 'lib'))
	libpath.append(os.path.join(varenv['boost_path'], 'lib'))
	libpath.append(os.path.join(varenv['openssl_path'], 'lib'))

	# Add winsock library for MS Windows
	libs.append('gdi32')
	libs.append('ws2_32')
	libs.append('iconv')

	if varenv['without_libxmlsec'] == '0':
		cxxflags.append('-DXMLSEC_STATIC')
		syspath.append(os.path.join(varenv['libxmlsec_path'], 'include', 'xmlsec1'))
		libpath.append(os.path.join(varenv['libxmlsec_path'], 'lib'))

# MAC OS specific adjustments
elif sys.platform == 'darwin':
	syspath.append(os.path.join(varenv['libxml2_path'], 'include', 'libxml2'))

	libpath.append(os.path.join(varenv['libxml2_path'], 'lib'))

	# universal library
	cxxflags.append('-arch')
	cxxflags.append('i386')
	cxxflags.append('-arch')
	cxxflags.append('x86_64')

	libs.append('iconv')

	if varenv['without_libxmlsec'] == '0':
		syspath.append(os.path.join(varenv['libxmlsec_path'], 'include', 'xmlsec1'))
		libpath.append(os.path.join(varenv['libxmlsec_path'], 'lib'))

# UNIX OSes specific adjustments
else:
	syspath.append(os.path.join(varenv['libxml2_path'], 'include', 'libxml2'))

	if varenv['without_libxmlsec'] == '0':
		syspath.append(os.path.join(varenv['libxmlsec_path'], 'include', 'xmlsec1'))

	# FreeBSD specific adjustments
	if sys.platform.startswith('freebsd'):
		cxxflags.remove('-pedantic')
		libpath.append(os.path.join(varenv['boost_path'], 'lib'))
		syspath.append(os.path.join(varenv['boost_path'], 'include'))

		if varenv['without_libxmlsec'] == '0':
			libpath.append(os.path.join(varenv['libxmlsec_path'], 'lib'))

for path in syspath:
	cxxflags.append('-isystem' + os.path.abspath(path))

def LoadVariables():
	return _variables

def SaveVariables(env):
	_variables.Save('variables.cache', env)

def GetVar(variable):
	return varenv[variable]

