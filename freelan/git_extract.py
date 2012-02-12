"""A Git archive extracter."""

import re
import os

def git_extract_emitter(target, source, env):
    """The emitter"""

    if os.path.isdir(str(target[0])):
        target = [
            os.path.join(
                str(target[0]),
                os.path.basename(str(source[0])) + '-' + env['GIT_EXTRACT_REF'] + env['BUILDERS']['GitExtract'].suffix
            )
        ]

    return target, source

def git_extract_generator(target, source, env, for_signature):
    """The generator"""

    if len(target) != 1:
        raise SCons.Errors.BuildError(target, 'More than one target specified')

    if len(source) != 1:
        raise SCons.Errors.BuildError(target, 'More than one source specified')

    result = '%s archive --remote %s --prefix %s %s --format tar | %s > %s' % \
            (
                env['GIT_EXTRACT_TOOL'],
                str(source[0]),
                os.path.basename(str(target[0])).replace(env['BUILDERS']['GitExtract'].suffix, '') + '/',
                env['GIT_EXTRACT_REF'],
                env['GIT_EXTRACT_ZIP_TOOL'],
                str(target[0]),
            )

    return result

def generate(env):
    env.Append(GIT_EXTRACT_TOOL = 'git')
    env.Append(GIT_EXTRACT_ZIP_TOOL = 'gzip')
    env.Append(GIT_EXTRACT_REF = 'master')

    import SCons.Builder

    git_extract_builder = SCons.Builder.Builder(
        generator = git_extract_generator,
        emitter = git_extract_emitter,
        suffix = '.tar.gz',
    )

    env.Append(BUILDERS = {'GitExtract': git_extract_builder})

def exists(env):
    return env.Detect(env['GIT_EXTRACT_TOOL']) and env.Detect(env['GIT_EXTRACT_ZIP_TOOL'])
