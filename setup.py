#!/usr/bin/python

from distutils.core import setup

setup(
    name='freelan-build-tools',
    version='1.0',
    description='Freelan build tools',
    author='Julien Kauffmann',
    author_email='julien.kauffmann@freelan.org',
    url='http://github.com/ereOn/freelan-build-tools',
    packages=['freelan'],
    license='GPLv3',
    classifiers=[
        'Environment :: Console',
        'Intended Audience :: Developers',
        'Programming Language :: Python',
        'Operating System :: OS Independent',
        'Topic :: Software Development :: Build Tools',
    ],
)
