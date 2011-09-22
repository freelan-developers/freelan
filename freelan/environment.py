"""Build environment related functions and classes."""

import sys

if sys.platform.startswith('linux'):
    from posixenvironment import *
elif sys.platform.startswith('win32'):
    from ntenvironment import *
elif sys.platform.startswith('darwin'):
    from macenvironment import *
else:
    raise RuntimeError('Unsupported platform (%s)' % sys.platform)
