"""A INNO Setup builder for SCons"""

import re
import os

def uncomment(text):
    """Remove all INNO Setup comments from the specified text"""

    def replacer(match):
        s = match.group(0)
        if s.startswith(';'):
            return ''
        else:
            return s

    pattern = ';.*?$|"[^"]*"|\'[^"]*\''

    return re.sub(re.compile(pattern, re.DOTALL | re.MULTILINE), replacer, text)

def replace_defines(text, defines):
    """Replace defines in the specified text."""

    for define in defines.items():
        text = text.replace('{#%s}' % define[0], define[1])

    return text

def innosetup_scanner(node, env, path):
    """The scanner"""

    content = replace_defines(node.get_contents(), env['ISCC_DEFINES'])

    result = []
    return result

def innosetup_emitter(source, target, env):
    """The emitter"""

    content = replace_defines(source[0].get_contents(), env['ISCC_DEFINES'])

    return (target, source)

def innosetup_generator(source, target, env, for_signature):
    """The generator"""

    def to_define_option(item):
        if item[1]:
            return '"/d%s=%s"' % (item[0], item[1])
        else:
            return '"/d%s"' % item[0]

    result = '%s %s %s "/o%s" "/f%s" %s' % \
            (
                env['ISCC'],
                ' '.join(env['ISCC_FLAGS']),
                ' '.join([to_define_option(x) for x in env['ISCC_DEFINES'].items()]),
                os.path.dirname(str(target[0])),
                os.path.splitext(os.path.basename(str(target[0])))[0],
                source[0]
            )

    return result

def generate(env):
    env.Append(ISCC = 'iscc')
    env.Append(ISCC_PATH = [])
    env.Append(ISCC_FLAGS = []);
    env.Append(ISCC_DEFINES = {});

    import SCons.Scanner

    env.Append(SCANNERS = SCons.Scanner.Scanner(
        function = innosetup_scanner,
        skeys = ['.iss'],
        path_function = SCons.Scanner.FindPathDirs('ISCC_PATH')
    ))

    import SCons.Builder

    innosetup_builder = SCons.Builder.Builder(
        generator = innosetup_generator,
        emitter = innosetup_emitter,
        suffix = '.exe',
        src_suffix = '.iss'
    )

    env.Append(BUILDERS = {'INNOSetup': innosetup_builder})

def exists(env):
    return env.Detect(env['ISCC'])
