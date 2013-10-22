"""
The Fabric file.
"""

import os
import copy
import fnmatch
import urlparse
from glob import glob
from debian.deb822 import Deb822, Changes
from fabric.api import settings

CONFIGURATION_DIR = 'configuration'
ARCHIVES_DIR = 'archives'
SOURCES_DIR = 'sources'
BUILD_DIR = 'build'
REPOSITORY_DIR = 'repository'
OFFICIAL_REPOSITORY_DIR = 'repository.official'

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
    'freelan': {
        'tag': '1.1',
        'depends': [
            'freelan-buildtools',
            'libfreelan',
        ],
    },
    'flask': {
        'tag': '0.10.1',
        'provider': github(user='mitsuhiko'),
    },
    'flask-sqlalchemy': {
        'tag': '1.0',
        'provider': github(user='mitsuhiko'),
        'depends': [
            'flask',
        ],
    },
    'flask-login': {
        'tag': '0.2.6',
        'provider': github(user='maxcountryman'),
        'depends': [
            'flask',
        ],
    },
    'simplekv': {
        'tag': '0.5',
        'provider': github(user='mbr'),
    },
    'itsdangerous': {
        'tag': '0.23',
        'depends': [],
        'provider': github(user='mitsuhiko'),
    },
    'flask-kvsession': {
        'tag': '0.4',
        'depends': [
            'flask',
            'simplekv',
            'itsdangerous',
        ],
        'provider': github(user='mbr'),
    },
    'flask-gravatar': {
        'tag': '0.3.0',
        'provider': github(user='zzzsochi'),
        'depends': [
            'flask',
        ],
    },
    'flask-wtf': {
        'tag': '0.8.4',
        'upstream_tag': 'v0.8.4',
        'provider': github(user='ajford'),
        'depends': [
            'flask',
        ],
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

def __get_dependencies(repository, flat=False):
    """
    Get all the dependencies for the given repository, as a tree.
    """

    if not repository in REPOSITORIES:
        return

    tree = {repository: {}}

    for dependency in REPOSITORIES[repository].get('depends', []):
        tree[repository].update(__get_dependencies(dependency))

    if flat:

        def flatten(node, list_tree=[]):
            for key, value in node.items():
                flatten(value)

                if not key in list_tree:
                    list_tree.append(key)

            return list_tree

        return flatten(tree)

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
        'official_repository_path': OFFICIAL_REPOSITORY_DIR,
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

    for architecture in ['i386', 'amd64', 'armhf']:
        for distribution in ['wheezy-backports', 'unstable']:
            copy_file('%configuration_path%' + '/pbuilderrc-%s-%s' % (distribution, architecture), '~/.pbuilderrc-%s-%s' % (distribution, architecture))

def cowbuilder(override=False, distributions=None, architectures=None):
    """
    Create the cowbuilder environment.
    """

    if isinstance(architectures, basestring):
        architectures = [architectures]

    for architecture in architectures or ['i386', 'amd64', 'armhf']:
        for distribution in distributions or ['wheezy-backports', 'unstable']:
            basepath = '/var/cache/pbuilder/base-%s-%s.cow' % (distribution, architecture)

            if override:
                local('sudo rm -rf %s' % basepath)

            local('[ -d %(basepath)s ] && sudo cowbuilder --update --config ~/.pbuilderrc-%(distribution)s-%(architecture)s --basepath %(basepath)s || sudo cowbuilder --create --config ~/.pbuilderrc-%(distribution)s-%(architecture)s --basepath %(basepath)s' % { 'basepath': basepath, 'distribution': distribution, 'architecture': architecture })

def cowshell(distribution='unstable', architecture='amd64'):
    """
    Login into the cowbuilder environment.
    """

    basepath = '/var/cache/pbuilder/base-%s-%s.cow' % (distribution, architecture)
    local('[ -d %(basepath)s ] && sudo cowbuilder --login --config ~/.pbuilderrc-%(distribution)s-%(architecture)s --basepath %(basepath)s || echo "No cowbuilder environment found. Please create one with \'fab cowbuilder\' first."' % { 'basepath': basepath, 'distribution': distribution, 'architecture': architecture })

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
        local('reprepro -b . createsymlinks')

def buildpackage(unsigned=False, build_all=False):
    """
    Build a package.
    """

    options = __get_options()

    if build_all:
        sources_path = options['sources_path']

        for repository in REPOSITORIES:

            with lcd(os.path.join(sources_path, repository + '.debian')):
                has_backports_branch = bool(int(local('git show-ref --verify --quiet refs/heads/wheezy-backports && echo 1 || echo 0', capture=True).strip()))

                local('git checkout master')
                buildpackage(unsigned=unsigned)

                if has_backports_branch:
                    local('git checkout wheezy-backports')
                    buildpackage(unsigned=unsigned)
    else:
        build_path = options['build_path']

        local('mkdir -p %s' % build_path)

        current_branch = local('git rev-parse --abbrev-ref HEAD', capture=True).strip()

        if unsigned:
            local('git buildpackage -S -uc -us --git-debian-branch=%s' % current_branch)
        else:
            local('git buildpackage -S --git-debian-branch=%s' % current_branch)

def depbinary(unsigned=False, repository=None, no_prompt=True, force=False):
    """
    Build binary packages with their dependencies.
    """

    return binary(unsigned=unsigned, with_dependencies=True, repository=repository, no_prompt=no_prompt, force=force)

def binary(unsigned=False, with_dependencies=False, repository=None, no_prompt=False, force=False):
    """
    Build binary packages.
    """

    options = __get_options()

    sources_path = options['sources_path']
    build_path = options['build_path']
    sources_build_path = os.path.join(build_path, 'sources')
    binaries_build_path = os.path.join(build_path, 'binaries')
    source_packages = []

    for source_package_path in reversed(sorted(glob(os.path.join(sources_build_path, '*.dsc')))):
        source_package = Deb822(open(source_package_path))
        source_package.path = source_package_path
        source_packages.append(source_package)

    repository_path = options['repository_path']

    if not source_packages:
        warn('No source packages (*.dsc) found in "%s". Did you forget to call `fab buildpackage` ?' % sources_build_path)

    else:
        if not repository:
            current_dir = os.path.abspath(os.getcwd())

            if current_dir in (os.path.join(sources_path, '%s.debian') % x for x in REPOSITORIES):
                repository = os.path.splitext(os.path.basename(current_dir))[0]

        if with_dependencies:
            repositories = __get_dependencies(repository, flat=True)
        else:
            repositories = [repository]

        print 'About to build: %s' % ', '.join(repositories)

        for repository in repositories:
            print 'Upgrading the cowbuilder environments...'
            cowbuilder()

            repo_source_packages = [package for package in source_packages if package.get('Source') == repository]

            if len(repo_source_packages) > 1:
                if not no_prompt:
                    puts('Which package do you want to build ?')

                    for index, package in enumerate(repo_source_packages):
                        puts(indent('[%s]' % index + ' %(Source)s (%(Version)s)' % package, 2))

                    choice = prompt('Your choice or `n` to quit: ', validate=lambda x: x if (x.lower() == 'n') else int(x))
                    selected_source_packages = [repo_source_packages[choice]]
                else:
                    selected_source_packages = repo_source_packages

            else:
                selected_source_packages = repo_source_packages

                if not no_prompt and not prompt('You are about to build the only available source package: %(Source)s (%(Version)s)\n\nDo you want to proceed ? [y/N]: ' % selected_source_packages[0], validate=lambda x: x.lower() in ['y']):
                    puts('Aborting.')
                    selected_source_packages = []

            for source_package in selected_source_packages:
                package_name = '%(Source)s_%(Version)s' % source_package

                local_architecture = local('dpkg --print-architecture', capture=True)

                source_architecture = source_package.get('Architecture')

                if source_architecture == 'all':
                    architectures = [local_architecture]
                elif source_architecture == 'any':
                    architectures = ['i386', 'amd64', 'armhf']
                else:
                    architectures = [source_architecture]

                if '~' in source_package.get('Version'):
                    distribution = 'wheezy-backports'
                else:
                    distribution = 'unstable'

                for architecture in architectures:
                    target_changes_file = os.path.join(binaries_build_path, '%s_%s.changes' % (package_name, architecture))
                    deb_file = os.path.join(binaries_build_path, '%s_%s.deb' % (package_name, architecture))

                    puts('Checking for %s existence...' % deb_file)

                    if not force and os.path.isfile(deb_file):
                        puts('Not building %(Source)s (%(Version)s) as it was already built and the "force" option wasn\'t specified.' % source_package)
                    else:
                        puts('Building %(Source)s (%(Version)s)...' % source_package)

                        launcher = 'linux64' if (architecture == 'amd64') else 'linux32'

                        basepath = '/var/cache/pbuilder/base-%s-%s.cow' % (distribution, architecture)
                        local(
                            'sudo %(launcher)s cowbuilder --configfile ~/.pbuilderrc-%(distribution)s-%(architecture)s --build %(source_package_path)s --debbuildopts "-sa" --basepath %(basepath)s' % {
                                'launcher': launcher,
                                'source_package_path': source_package.path,
                                'basepath': basepath,
                                'distribution': distribution,
                                'architecture': architecture,
                            }
                        )

                        with settings(warn_only=True):
                            local(
                                'reprepro -b %(repository_path)s include %(distribution)s %(target_changes_file)s' % {
                                    'repository_path': repository_path,
                                    'distribution': distribution,
                                    'target_changes_file': target_changes_file,
                                }
                            )

def mount_official_repository():
    """
    Mount the official repository.
    """

    options = __get_options()

    official_repository_path = options['official_repository_path']

    local('mkdir -p %(path)s && sshfs -o idmap=user freelan@ftp.freelan.org:debian %(path)s' % { 'path': official_repository_path })

def update_official_repository():
    """
    Update the official repository.
    """

    options = __get_options()

    official_repository_path = options['official_repository_path']
    build_path = options['build_path']
    binaries_build_path = os.path.join(build_path, 'binaries')

    changes_files = []

    for root, dirs, files in os.walk(binaries_build_path):
        changes_files.extend(os.path.join(root, _file) for _file in fnmatch.filter(files, '*.changes'))

    with lcd(official_repository_path):
        with settings(warn_only=True):

            for changes_file in changes_files:
                changes = Changes(open(changes_file))
                distribution = changes['Distribution']

                local(
                    'reprepro -b . include %(distribution)s %(changes_file)s' % {
                        'distribution': distribution,
                        'changes_file': changes_file,
                    }
                )

        local('reprepro -b . createsymlinks')
