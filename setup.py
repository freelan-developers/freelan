from setuptools import (
    find_packages,
    setup,
)

import os
import platform

project_root = os.path.dirname(os.path.realpath(__file__))
project_bin_dir = os.path.join(project_root, 'install', 'bin')
project_include_dir = os.path.join(project_root, 'install', 'include')
project_lib_dir = os.path.join(project_root, 'install', 'lib')


def prepend_paths(name, *values):
    """
    Add a list of paths to a specified path-like environment variable.

    :param name: The name of the environment variable to modify.
    :param values: A list of paths to prepend to the environment variable.
    """
    paths = values + tuple(os.environ.get(name, '').split(os.pathsep))
    os.environ[name] = os.pathsep.join(paths)


if platform.system() == 'Windows':
    # On Windows we detect the compiler and add it to the path if it isn't
    # there already.
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

    prepend_paths('PATH', project_bin_dir)
    prepend_paths('INCLUDE', project_include_dir)
    prepend_paths('LIB', project_lib_dir)
else:
    prepend_paths('PATH', project_bin_dir)
    prepend_paths('CPATH', project_include_dir)
    prepend_paths('LIBRARY_PATH', project_lib_dir)

    if platform.system() == 'Darwin':
        prepend_paths('DYLD_LIBRARY_PATH', project_lib_dir)
    else:
        prepend_paths('LD_LIBRARY_PATH', project_lib_dir)

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
