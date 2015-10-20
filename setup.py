from setuptools import (
    find_packages,
    setup,
)

import os
import platform


def prepend_paths(name, *values):
    """
    Add a list of paths to a specified path-like environment variable.

    :param name: The name of the environment variable to modify.
    :param values: A list of paths to prepend to the environment variable.
    """
    paths = values + tuple(os.environ.get(name, '').split(os.pathsep))
    os.environ[name] = os.pathsep.join(paths)


# On Windows we detect the compiler and add it to the path if it isn't there
# already.
if platform.system() == 'Windows':
    from shutil import which
    from distutils.ccompiler import new_compiler
    from distutils.sysconfig import customize_compiler

    if not which('cl'):
        compiler = new_compiler()
        customize_compiler(compiler)
        compiler.initialize()

        prepend_paths('PATH', os.path.dirname(compiler.cc))
        prepend_paths('INCLUDE', *compiler.include_dirs)
        prepend_paths('LIB', *compiler.library_dirs)

setup(
    name='pyfreelan',
    version='0.0.1',
    author='Julien Kauffmann',
    author_email='julien.kauffmann@freelan.org',
    url='http://www.freelan.org',
    description='A Python layer around the FreeLAN API.',
    setup_requires=["cffi>=1.1.2"],
    install_requires=[
        "cffi>=1.1.2",
        "six>=1.9.0",
    ],
    cffi_modules=["pyfreelan/api/_build.py:ffi"],
    packages=find_packages(
        exclude=[
            'pyfreelan.api._build',
            'tests',
        ],
    ),
    ext_package='pyfreelan',
    test_suite='tests',
    entry_points={
        'nose.plugins.0.10': [
            'flmem = pyfreelan_nose_plugin:FreeLANMemory',
        ],
    },
)
