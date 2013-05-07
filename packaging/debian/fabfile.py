"""
The Fabric file.
"""

import os
import copy

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

ARCHIVES_OUTPUT_DIR = 'archives'
SOURCES_DIR = 'sources'

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

# Below are the fabric commands

from fabric.api import *

def archives():
    """
    Make archives of the git repositories.
    """

    archives_path = os.path.abspath(os.path.join(os.path.dirname(env.real_fabfile), ARCHIVES_OUTPUT_DIR))

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

    sources_path = os.path.abspath(os.path.join(os.path.dirname(env.real_fabfile), SOURCES_DIR))
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
