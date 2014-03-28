"""Build environment related functions and classes."""

import sys

if sys.platform.startswith('linux'):
    from posixenvironment import PosixEnvironment as Environment
elif sys.platform.startswith('win32'):
    from ntenvironment import NtEnvironment as Environment
elif sys.platform.startswith('darwin'):
    from macenvironment import MacEnvironment as Environment
else:
    raise RuntimeError('Unsupported platform (%s)' % sys.platform)
