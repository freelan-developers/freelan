"""
The SConstruct file.

Works on all UNIX-like operating systems.
"""

import os

from fnmatch import fnmatch


AddOption(
    '--prefix',
    dest='prefix',
    type='string',
    nargs=1,
    action='store',
    metavar='DIR',
    help='The installation prefix.',
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

        for flag in [
            'CXX',
            'CXXFLAGS',
            'AR',
            'ARFLAGS',
            'LINK',
            'LINKFLAGS',
        ]:
            if flag in os.environ:
                self[flag] = os.environ[flag]

        self.debug = debug

        self.Append(CXXFLAGS=['--std=c++11'])
        self.Append(CXXFLAGS=['-Wall'])
        self.Append(CXXFLAGS=['-Wextra'])
        self.Append(CXXFLAGS=['-Werror'])
        self.Append(CXXFLAGS=['-pedantic'])
        self.Append(CXXFLAGS=['-Wshadow'])
        self.Append(CXXFLAGS=['-Wno-long-long'])
        self.Append(CXXFLAGS=['-Wno-uninitialized'])
        self.Append(CXXFLAGS=['-Wno-strict-aliasing'])

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


envs = {
    'release': FreelanEnvironment(debug=False),
    'debug': FreelanEnvironment(debug=True),
}

for dirname, env in envs.items():
    libraries, includes, apps = SConscript('SConscript', exports='env', variant_dir=os.path.join('build', dirname))
