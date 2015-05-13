from distutils.core import setup

setup(
    name='pyfreelan',
    author='Julien Kauffmann',
    author_email='julien.kauffmann@freelan.org',
    packages=[
        'pyfreelan',
        'pyfreelan.api',
    ],
    test_suite='tests',
    entry_points={
        'nose.plugins.0.10': [
            'fmemory = pyfreelan.nose_plugin:FreeLANMemory',
        ],
    },
)
