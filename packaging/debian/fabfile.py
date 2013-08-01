"""
The Fabric file.
"""

import os
import copy
import urlparse
from glob import glob

CONFIGURATION_DIR = 'configuration'
ARCHIVES_DIR = 'archives'
SOURCES_DIR = 'sources'
BUILD_DIR = 'build'
REPOSITORY_DIR = 'repository'

class LocalProvider(object):

    """
    The local provider.
    """

    def __call__(self, repository, tag, target):

        """
        Get the archive from a local repository.
        """

        repository_path = os.path.abspath(os.path.join(os.path.dirname(env.real_fabfile), '..', '..', repository))

        if tag:
            with lcd(repository_path):
                local('git archive %(tag)s | gzip > %(target)s' % {
                    'tag': tag,
                    'target': target,
                })

class GithubProvider(object):

    """
    A Github provider.
    """

    def __init__(self, user):

        self.user = user

    def __call__(self, repository, tag, target):

        """
        Download the archive from github.
        """

        url = urlparse.urlunparse([
            'https',
            'github.com',
            '%(user)s/%(repository)s/archive/%(tag)s.tar.gz' % {
                'user': self.user,
                'repository': repository,
                'tag': tag,
            },
            '',
            '',
            '',
        ])

        download(url=url, target=target)

github = GithubProvider

REPOSITORIES = {
    'freelan-buildtools' : {
        'tag': '1.2',
    },
    'libcryptoplus' : {
        'tag': '2.0',
        'depends': [
            'freelan-buildtools',
        ],
    },
    'libiconvplus' : {
        'tag': '1.1',
        'depends': [
            'freelan-buildtools',
        ],
    },
    'libkfather': {
        'tag': '1.0',
        'depends': [
            'freelan-buildtools',
            'libiconvplus',
        ],
    },
    'libfscp': {
        'tag': '2.1',
        'depends': [
            'freelan-buildtools',
            'libkfather',
            'libcryptoplus',
        ],
    },
    'libasiotap': {
        'tag': '2.0',
        'depends': [
            'freelan-buildtools',
        ],
    },
    'libfreelan': {
        'tag': '2.1',
        'depends': [
            'freelan-buildtools',
            'libfscp',
            'libasiotap',
        ],
    },
    'flask-sqlalchemy': {
        'tag': '1.0',
        'provider': github(user='mitsuhiko'),
    },
    'flask-login': {
        'tag': '0.2.6',
        'provider': github(user='maxcountryman'),
    },
    'simplekv': {
        'tag': '0.5',
        'provider': github(user='mbr'),
    },
    'flask-kvsession': {
        'tag': '0.4',
        'depends': [
            'simplekv',
        ],
        'provider': github(user='mbr'),
    },
    'flask-gravatar': {
        'tag': '0.3.0',
        'provider': github(user='zzzsochi'),
    },
    'flask-wtf': {
        'tag': '0.8.4',
        'upstream_tag': 'v0.8.4',
        'provider': github(user='ajford'),
    },
    'freelan-server': {
        'tag': '1.0',
        'depends': [
            'flask-sqlalchemy',
            'flask-login',
            'flask-kvsession',
            'flask-gravatar',
            'flask-wtf',
        ],
    },
}

def __get_dependencies(repository):
    """
    Get all the dependencies for the given repository, as a tree.
    """

    tree = {repository: {}}

    for dependency in REPOSITORIES[repository].get('depends', []):
        tree[repository].update(__get_dependencies(dependency))

    return tree

def __get_ordered_repositories():
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

def __get_options():
    """
    Get a dictionary of all the sensible paths.
    """

    paths_aliases = {
        'configuration_path': CONFIGURATION_DIR,
        'archives_path': ARCHIVES_DIR,
        'sources_path': SOURCES_DIR,
        'build_path': BUILD_DIR,
        'repository_path': REPOSITORY_DIR,
    }

    def get_dir_path(_dir):
        return os.path.abspath(os.path.join(os.path.dirname(env.real_fabfile), _dir))

    return dict(map(lambda x: (x[0], get_dir_path(x[1])), paths_aliases.items()))

def copy_file(source, target):
    """
    Copy a file from source to target, replacing content with options if needed.
    """

    options = __get_options()

    target = os.path.expanduser(target)

    for option, value in options.items():
        source = source.replace('%%%s%%' % option, value)
        target = target.replace('%%%s%%' % option, value)

    print('Copying %s to %s ...' % (source, target))

    with open(source) as original_file:
        content = original_file.read()

    for option, value in options.items():
        content = content.replace('%%%s%%' % option, value)

    with open(target, 'w') as target_file:
        target_file.write(content)

# Below are the fabric commands

from fabric.api import *
from fabric.utils import *
from fabric.operations import *

def download(url, target):

    """
    Download stuff from the web.
    """

    local('wget -O %(target)s %(url)s' % {
        'url': url,
        'target': target,
    })

def archives(override=False):
    """
    Make archives of the git repositories.
    """

    options = __get_options()

    archives_path = options['archives_path']

    local('mkdir -p %s' % archives_path)

    for repository, attributes in REPOSITORIES.items():
        provider = attributes.get('provider', LocalProvider())
        tag = attributes.get('tag')
        upstream_tag = attributes.get('upstream_tag', tag)
        target = '%(output_dir)s/%(repository)s_%(tag)s.orig.tar.gz' % {
            'repository': repository,
            'tag': tag,
            'output_dir': archives_path,
        }

        if not os.path.isfile(target) or override:
            provider(repository=repository, tag=upstream_tag, target=target)
        else:
            print 'Not downloading already existing archive: %s' % target

def sources(override=False):
    """
    Generate the source packages.
    """

    options = __get_options()

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

            local(' || '.join([
                '[ -d %(repository)s.debian ]',
                '(%s)' % ' && '.join([
                    'git clone %(uri)s',
                    'cd %(repository)s.debian',
                    'git checkout upstream',
                    'git checkout pristine-tar',
                    'git checkout master',
                ]),
            ]) % {
                'uri': vcs_uri,
                'repository': repository,
            })

def configure():
    """
    Copy and fill the configuration files were appropriate.
    """

    copy_file('%configuration_path%/gbp.conf', '~/.gbp.conf')
    copy_file('%configuration_path%/pbuilderrc', '~/.pbuilderrc')

def cowbuilder(override=False):
    """
    Create the cowbuilder environment.
    """

    if override:
        local('sudo rm -rf /var/cache/pbuilder/base.cow')

    local('[ -d /var/cache/pbuilder/base.cow ] && sudo cowbuilder --update --config ~/.pbuilderrc || sudo cowbuilder --create --config ~/.pbuilderrc ')

def cowshell():
    """
    Login into the cowbuilder environment.
    """

    local('[ -d /var/cache/pbuilder/base.cow ] && sudo cowbuilder --login --config ~/.pbuilderrc || echo "No cowbuilder environment found. Please create one with \'fab cowbuilder\' first."')

def repository(override=False):
    """
    Initializes the Debian repository.
    """

    options = __get_options()

    repository_path = options['repository_path']

    with lcd(repository_path):
        if override:
            local('git clean -f -x -d')

        local('reprepro -b . export')

def buildpackage(unsigned=False):
    """
    Build a package.
    """

    options = __get_options()

    build_path = options['build_path']

    local('mkdir -p %s' % build_path)

    if unsigned:
        local('git buildpackage -S -uc -us')
    else:
        local('git buildpackage -S')

def depbinary(unsigned=False,repository=None):
    """
    Build binary packages with their dependencies.
    """

    return binary(unsigned=unsigned, with_dependencies=True, repository=repository)

def binary(unsigned=False,with_dependencies=False,repository=None):
    """
    Build binary packages.
    """

    options = __get_options()

    sources_path = options['sources_path']
    build_path = options['build_path']
    sources_build_path = os.path.join(build_path, 'sources')
    binaries_build_path = os.path.join(build_path, 'binaries')
    source_packages = list(reversed(sorted(glob(os.path.join(sources_build_path, '*.dsc')))))
    repository_path = options['repository_path']

    if not source_packages:
        warn('No source packages (*.dsc) found in "%s". Did you forget to call `fab buildpackage` ?' % sources_build_path)

    else:
        if repository:
            current_repository = repository
        else:
            current_dir = os.path.abspath(os.getcwd())

            if current_dir in (os.path.join(sources_path, '%s.debian') % x for x in REPOSITORIES):
                current_repository = os.path.splitext(os.path.basename(current_dir))[0]

        if current_repository:
            source_packages = filter(lambda x: os.path.basename(x).startswith(current_repository + '_'), source_packages)

            if with_dependencies:

                for dependency in __get_dependencies(current_repository)[current_repository]:
                    print 'First building `%s` for `%s`...' % (dependency, current_repository)
                    binary(unsigned=unsigned, with_dependencies=with_dependencies, repository=dependency)

        if len(source_packages) > 1:
            puts('Which package do you want to build ?')

            for item in enumerate(source_packages):
                puts(indent('[%s] %s' % item, 2))

            choice = prompt('Your choice or `n` to quit: ', validate=lambda x: x if (x.lower() == 'n') else int(x))
            source_package = source_packages[choice]

        else:
            source_package = source_packages[0]

            if not prompt('You are about to build the only available source package: %s\n\nDo you want to proceed ? [y/N]: ' % source_package, validate=lambda x: x.lower() in ['y']):
                puts('Aborting.')
                source_package = None

        if source_package:
            package_name = os.path.splitext(os.path.basename(source_package))[0]
            architecture = local('dpkg --print-architecture', capture=True)
            target_changes_file = os.path.join(binaries_build_path, '%s_%s.changes' % (package_name, architecture))

            puts('Building %s...' % source_package)
            local('sudo cowbuilder --configfile ~/.pbuilderrc --build %(source_package)s --debbuildopts "-sa" && reprepro -b %(repository_path)s include %(distribution)s %(target_changes_file)s ' % {
                'source_package': source_package,
                'repository_path': repository_path,
                'distribution': 'unstable',
                'target_changes_file': target_changes_file,
            })
