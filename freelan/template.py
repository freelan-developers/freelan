"""A template file builder."""

import re
import os
import SCons

def template_emitter(target, source, env):
    """A template emitter function."""

    env.Depends(target, SCons.Node.Python.Value(env['TEMPLATE_VALUES']))

    return target, source

def template_replace(target, source, env):
    """Generate a file from a template"""

    if len(target) != 1:
        raise SCons.Errors.BuildError(target, 'More than one target specified')

    if len(source) != 1:
        raise SCons.Errors.BuildError(target, 'More than one source specified')

    content = open(str(source[0])).read()

    for key, value in env['TEMPLATE_VALUES'].items():
        if isinstance(value, str):
            content = content.replace('{{' + key + '}}', value)
        elif isinstance(value, list):
            content = content.replace('{{' + key + '}}', ', '.join(value))

    f = open(str(target[0]), 'w')
    f.write(content)
    f.close()

def generate(env):
    env.Append(TEMPLATE_VALUES = {})

    import SCons.Builder

    template_builder = SCons.Builder.Builder(
        emitter = template_emitter,
        action = template_replace,
        src_suffix = '.template',
    )

    env.Append(BUILDERS = {'Template': template_builder})

def exists(env):
    return True
