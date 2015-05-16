from distutils.core import setup

setup(
    name='pyfreelan',
    version='0.0.1',
    author='Julien Kauffmann',
    author_email='julien.kauffmann@freelan.org',
    packages=[
        'pyfreelan',
        'pyfreelan.api',
    ],
    test_suite='tests',
    entry_points={
        'nose.plugins.0.10': [
            'flmem = pyfreelan.nose_plugin:FreeLANMemory',
        ],
    },
)
