from setuptools import (
    find_packages,
    setup,
)

setup(
    name='pyfreelan',
    version='0.0.1',
    author='Julien Kauffmann',
    author_email='julien.kauffmann@freelan.org',
    url='http://www.freelan.org',
    description='A Python layer around the FreeLAN API.',
    setup_requires=["cffi>=1.0.0"],
    install_requires=["cffi>=1.0.0"],
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
