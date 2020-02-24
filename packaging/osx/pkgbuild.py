"""A SCons builder for pkgbuild"""

import SCons.Warnings
import SCons.Errors


def pkgbuild_emitter(target, source, env):
    """The emitter"""

    env.Depends(target, env['PKGBUILD_OPTIONS'])
    env.Depends(target, env['PKGBUILD_SCRIPTS'])

    return (target, source)


def pkgbuild_generator(target, source, env, for_signature):
    """The generator"""

    options = env['PKGBUILD_OPTIONS'].value
    options_str = ' '.join(
        '--%s %s' % (key, value)
        for key, value in options.items()
    )

    if env['PKGBUILD_SCRIPTS']:
        options_str = options_str + ' --scripts $PKGBUILD_SCRIPTS'

    return '{executable} {options_str} --root $SOURCE $TARGET'.format(
        executable=env['PKGBUILD'],
        options_str=options_str,
    )


class PkgBuildNotFound(SCons.Warnings.Warning):
    pass


def detect(env):
    try:
        return env['PKGBUILD']
    except KeyError:
        pass

    pkgbuild = env.WhereIs('pkgbuild')

    if pkgbuild:
        return pkgbuild

    raise SCons.Errors.StopError(
        PkgBuildNotFound,
        "Unable to find pkgbuild"
    )


def generate(env):
    env.Append(PKGBUILD=detect(env))
    env.Append(PKGBUILD_SCRIPTS=None)
    env.Append(PKGBUILD_OPTIONS=env.Value({}))

    import SCons.Builder

    pkgbuild_builder = SCons.Builder.Builder(
        generator=pkgbuild_generator,
        emitter=pkgbuild_emitter,
        suffix='.pkg',
    )

    env.Append(BUILDERS={'PkgBuild': pkgbuild_builder})


def exists(env):
    return env.Detect(env['PKGBUILD'])
