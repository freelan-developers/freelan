"""A SCons builder for plist files"""

import plistlib


def plist_action(target, source, env):
    for targ in target:
        plistlib.writePlist(source[0].value, targ.abspath)


def generate(env):
    import SCons.Builder

    plist_builder = SCons.Builder.Builder(
        action=plist_action,
        suffix='.plist',
    )

    env.Append(BUILDERS={'Plist': plist_builder})
