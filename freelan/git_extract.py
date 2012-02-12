"""A Git archive extracter."""

import re
import os

def git_extract_emitter(target, source, env):
    """The emitter"""

    if env['GIT_EXTRACT_FORMAT'] == 'gzip':
        suffix = '.tar.gz'
    elif env['GIT_EXTRACT_FORMAT'] == 'tar':
        suffix = '.tar'
    else:
        suffix = None

    if suffix:
        if os.path.isdir(str(target[0])):
            target = [
                os.path.join(
                    str(target[0]),
                    os.path.basename(str(source[0])) + '-' + env['GIT_EXTRACT_REF'] + suffix
                )
            ]

    return target, source

def git_extract_generator(target, source, env, for_signature):
    """The generator"""

    if len(target) != 1:
        raise SCons.Errors.BuildError(target, 'More than one target specified')

    if len(source) != 1:
        raise SCons.Errors.BuildError(target, 'More than one source specified')

    if env['GIT_EXTRACT_FORMAT'] == 'gzip':
        prefix = os.path.basename(str(target[0])).replace('.tar.gz', '') + '/'
        format_str = '%(git)s archive --remote %(source)s --prefix %(prefix)s %(ref)s --format tar | %(zip)s > %(target)s'
    elif env['GIT_EXTRACT_FORMAT'] == 'tar':
        prefix = os.path.basename(str(target[0])).replace('.tar', '') + '/'
        format_str = '%(git)s archive --remote %(source)s --prefix %(prefix)s %(ref)s --format tar > %(target)s'
    elif env['GIT_EXTRACT_FORMAT'] == 'folder':
        prefix = os.path.basename(str(target[0])) + '/'
        format_str = '%(git)s archive --remote %(source)s --prefix %(prefix)s %(ref)s --format tar > %(target)s.tar && %(tar)s xvf %(target)s.tar && rm %(target)s.tar'
    else:
        raise SCons.Errors.BuildError(target, 'Unknown format specified (%s)' % env['GIT_EXTRACT_FORMAT'])

    return format_str % \
            {
                'git': env['GIT_EXTRACT_TOOL'],
                'source': str(source[0]),
                'prefix': prefix,
                'ref': env['GIT_EXTRACT_REF'],
                'zip': env['GIT_EXTRACT_ZIP_TOOL'],
                'target': str(target[0]),
                'tar': env['GIT_EXTRACT_TAR_TOOL'],
            }

def generate(env):
    env.Append(GIT_EXTRACT_TOOL = 'git')
    env.Append(GIT_EXTRACT_TAR_TOOL = 'tar')
    env.Append(GIT_EXTRACT_ZIP_TOOL = 'gzip')
    env.Append(GIT_EXTRACT_REF = 'master')
    env.Append(GIT_EXTRACT_FORMAT = 'gzip')

    import SCons.Builder

    git_extract_builder = SCons.Builder.Builder(
        generator = git_extract_generator,
        emitter = git_extract_emitter,
    )

    env.Append(BUILDERS = {'GitExtract': git_extract_builder})

def exists(env):
    return env.Detect(env['GIT_EXTRACT_TOOL']) and env.Detect(env['GIT_EXTRACT_ZIP_TOOL'])
