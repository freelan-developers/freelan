"""
The Fabric file.
"""

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

    local('mkdir -p %s' % ARCHIVES_OUTPUT_DIR)

    for repository, tags in ARCHIVES.items():
        for tag in tags:
            with lcd(repository):
                local('git archive %(tag)s | gzip > ../%(output_dir)s/%(repository)s_%(tag)s.orig.tar.gz' % {
                    'repository': repository,
                    'tag': tag,
                    'output_dir': ARCHIVES_OUTPUT_DIR,
                })
