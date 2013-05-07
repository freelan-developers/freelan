"""
The Fabric file.
"""

import os

ARCHIVES = {
    'libkfather': [
        '1.0',
    ],
}
ARCHIVES_OUTPUT_DIR = 'archives'

from fabric.api import *

def archives():
    """
    Make archives of the git repositories.
    """

    archives_path = os.path.abspath(os.path.join(ARCHIVES_OUTPUT_DIR))

    local('mkdir -p %s' % archives_path)

    for repository, tags in ARCHIVES.items():
        repository_path = os.path.abspath(os.path.join('..', '..', repository))

        for tag in tags:
            with lcd(repository_path):
                local('git archive %(tag)s | gzip > %(output_dir)s/%(repository)s_%(tag)s.orig.tar.gz' % {
                    'repository': repository,
                    'tag': tag,
                    'output_dir': archives_path,
                })
