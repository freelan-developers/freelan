"""
The Fabric file.
"""

import os
import copy

CONFIGURATION_DIR = 'configuration'
ARCHIVES_DIR = 'archives'
SOURCES_DIR = 'sources'
CHROOTS_DIR = 'chroots'
BUILD_DIR = 'build'
REPOSITORY_DIR = 'repository'
INCOMING_DIR = 'incoming'

REPOSITORIES = {
    'freelan-buildtools' : {
        'tags': [
            '1.0',
            '1.1',
            '1.2',
        ],
        'depends': [],
    },
    'libcryptoplus' : {
        'tags': [
            '1.0',
            '1.1',
            '1.2',
            '1.3',
            '2.0',
        ],
        'depends': [
            'freelan-buildtools',
        ],
    },
    'libkfather': {
        'tags': [
            '1.0',
        ],
        'depends': [
            'freelan-buildtools',
        ],
    },
}

DISTRIBUTIONS = [
    'sid',
]

CONFIGURATION_FILES = {
    'gbp.conf': '.gbp.conf',
    'mini-dinstall.conf': '.mini-dinstall.conf',
    'dput.cf': '.dput.cf',
}

def get_ordered_repositories():
    """
    Get a list of the repositories ordered by dependencies.

    The less dependent repositories go first.
    """

    result = []
    repositories = dict((repository, copy.deepcopy(attributes.get('depends', []))) for repository, attributes in REPOSITORIES.items())

    while repositories:
        extracted = [repository for repository, depends in repositories.items() if not depends]

        if not extracted:
            raise RuntimeError('Cyclic dependency')

        for x in extracted:
            del repositories[x]

        result.extend(extracted)

        for depends in repositories.values():
            depends[:] = [depend for depend in depends if depend not in extracted]

    print result
    return result

def get_options():
    """
    Get a dictionary of all the sensible paths.
    """

    paths_aliases = {
        'configuration_path': CONFIGURATION_DIR,
        'archives_path': ARCHIVES_DIR,
        'sources_path': SOURCES_DIR,
        'chroots_path': CHROOTS_DIR,
        'build_path': BUILD_DIR,
        'repository_path': REPOSITORY_DIR,
        'incoming_path': INCOMING_DIR,
    }

    def get_dir_path(_dir):
        return os.path.abspath(os.path.join(os.path.dirname(env.real_fabfile), _dir))

    return dict(map(lambda x: (x[0], get_dir_path(x[1])), paths_aliases.items()))

# Below are the fabric commands

from fabric.api import *

def archives():
    """
    Make archives of the git repositories.
    """

    options = get_options()

    archives_path = options['archives_path']

    local('mkdir -p %s' % archives_path)

    for repository, attributes in REPOSITORIES.items():
        repository_path = os.path.abspath(os.path.join(os.path.dirname(env.real_fabfile), '..', '..', repository))

        for tag in attributes.get('tags', []):
            with lcd(repository_path):
                local('git archive %(tag)s | gzip > %(output_dir)s/%(repository)s_%(tag)s.orig.tar.gz' % {
                    'repository': repository,
                    'tag': tag,
                    'output_dir': archives_path,
                })

def sources(override=False):
    """
    Generate the source packages.
    """

    options = get_options()

    sources_path = options['sources_path']
    vcs_uri_pattern = 'git@github.com:freelan-developers/%(repository)s.debian.git'

    local('mkdir -p %s' % sources_path)

    for repository in REPOSITORIES:

        vcs_uri = vcs_uri_pattern % {
            'repository': repository,
        }

        with lcd(sources_path):
            if override:
                local('rm -rf %s.debian' % repository)

            local('[ -d %(repository)s.debian ] || git clone %(uri)s' % {
                'uri': vcs_uri,
                'repository': repository,
            })

def chroots(override=False):
    """
    Create chroots.
    """

    options = get_options()

    chroots_path = options['chroots_path']

    local('mkdir -p %s' % chroots_path)

    for distribution in DISTRIBUTIONS:
        with lcd(chroots_path):
            if override:
                local('rm -rf %s' % distribution)

            local('[ -d %(distribution)s ] || cowbuilder --create --basepath %(distribution)s' % {
                'distribution': distribution,
            })

            local('[ -d %(distribution)s ] || cowbuilder --update --basepath %(distribution)s' % {
                'distribution': distribution,
            })

def configure():
    """
    Copy and populate the configuration file.
    """

    options = get_options()

    configuration_path = options['configuration_path']
    home_path = os.path.expanduser('~')

    for source, target in CONFIGURATION_FILES.items():
        source_path = os.path.join(configuration_path, source)
        target_path = os.path.join(home_path, target)

        print('Copying %s to %s ...' % (source_path, target_path))

        with open(source_path) as original_file:
            content = original_file.read()

        for option, value in options.items():
            content = content.replace('%%%s%%' % option, value)

        with open(target_path, 'w') as target_file:
            target_file.write(content)

def repository():
    """
    Create and/or update the repository.
    """

    options = get_options()

    incoming_path = options['incoming_path']
    repository_path = options['repository_path']

    local('mkdir -p %s' % incoming_path)

    local('[ -d %(repository)s ] || mini-dinstall -b %(repository)s' % {
        'repository': repository_path,
    })
