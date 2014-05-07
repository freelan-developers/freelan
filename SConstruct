"""
The SConstruct file.

Works on all UNIX-like operating systems.
"""

import os
import sys

from fnmatch import fnmatch

# This file is local.
from defines import Defines


AddOption(
    '--prefix',
    dest='prefix',
    default='./install',
    type='string',
    nargs=1,
    action='store',
    metavar='DIR',
    help='The installation prefix.',
)

AddOption(
    '--mode',
    dest='mode',
    nargs=1,
    action='store',
    choices=('all', 'debug', 'release'),
    default='all',
    help='The compilation mode.',
)


class FreelanEnvironment(Environment):
    """
    A freelan specific environment class.
    """

    def __init__(self, debug, **kwargs):
        """
        Initialize the environment.

        `debug` is a boolean value that indicates whether to set debug flags in the environment.
        """

        super(FreelanEnvironment, self).__init__(**kwargs)

        self.defines = Defines()
        self.defines.register_into(self)

        for flag in [
            'CXX',
            'AR',
            'LINK',
        ]:
            if flag in os.environ:
                self[flag] = os.environ[flag]

        for flag in [
            'CXXFLAGS',
            'ARFLAGS',
            'LINKFLAGS',
        ]:
            if flag in os.environ:
                self[flag] = Split(os.environ[flag])

        self.debug = debug

        if os.path.basename(self['CXX']) == 'clang++':
            self.Append(CXXFLAGS=['-Qunused-arguments'])
            self.Append(CXXFLAGS=['-fcolor-diagnostics'])
        elif os.path.basename(self['CXX']).endswith('g++'):
            self.Append(CXXFLAGS=['-Wno-missing-field-initializers'])

        self.Append(CXXFLAGS=['--std=c++11'])
        self.Append(CXXFLAGS=['-Wall'])
        self.Append(CXXFLAGS=['-Wextra'])
        self.Append(CXXFLAGS=['-Werror'])
        self.Append(CXXFLAGS=['-pedantic'])
        self.Append(CXXFLAGS=['-Wshadow'])

        if 'OPENWRT_BUILD' in os.environ:
            self['ENV'] = os.environ
            self.arch = os.environ['ARCH']
        else:
            if sys.platform.startswith('darwin'):
                self.Append(CXXFLAGS=['-arch', 'x86_64'])
                self.Append(CXXFLAGS=['-DBOOST_ASIO_DISABLE_KQUEUE'])

            if self.debug:
                self.Append(CXXFLAGS=['-g'])
                self.Append(CXXFLAGS='-DFREELAN_DEBUG=1')
            else:
                self.Append(CXXFLAGS='-O3')

    def RGlob(self, path, patterns=None):
        """
        Returns a list of file objects that match the specified patterns.
        """

        path = unicode(path)

        if isinstance(patterns, basestring):
            patterns = [patterns]

        result = []
        abspath = Dir(path).srcnode().abspath

        for root, ds, fs in os.walk(abspath):
            prefix = os.path.relpath(root, abspath)
            for f in fs:
                if not patterns or any(fnmatch(f, pattern) for pattern in patterns):
                    result.append(File(os.path.join(path, prefix, f)))

        return result

    def RInstall(self, target, source, patterns=None):
        """
        Install a directory, keeping its structure.
        """

        files = self.RGlob(source, patterns)
        result = []

        for f in files:
            result.append(self.Install(os.path.join(str(target), os.path.dirname(str(f))), f))

        return result

    def SymLink(self, target, source):
        def create_symlink(target, source, env):
            os.symlink(os.path.abspath(str(source[0])), os.path.abspath(str(target[0])))

        return self.Command(target, source, create_symlink)


mode = GetOption('mode')
prefix = GetOption('prefix')

if mode in ('all', 'release'):
    env = FreelanEnvironment(debug=False)
    libraries, includes, apps, samples = SConscript('SConscript', exports='env', variant_dir=os.path.join('build', 'release'))
    install = env.Install(os.path.join(prefix, 'bin'), apps)
    Alias('install', install)
    Alias('apps', apps)
    Alias('samples', samples)
    Alias('all', install + apps + samples)

if mode in ('all', 'debug'):
    env = FreelanEnvironment(debug=True)
    libraries, includes, apps, samples = SConscript('SConscript', exports='env', variant_dir=os.path.join('build', 'debug'))
    Alias('apps', apps)
    Alias('samples', samples)
    Alias('all', apps + samples)

Default('install')
