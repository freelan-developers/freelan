"""A NSIS builder for SCons"""

import re
import os

def uncomment(text):
    """Remove all NSIS comments from the specified text"""

    def replacer(match):
        s = match.group(0)
        if s.startswith(';') or s.startswith('#'):
            return ''
        else:
            return s

    pattern = '#.*?$|;.*?$|"[^"]*"|\'[^"]*\''

    return re.sub(re.compile(pattern, re.DOTALL | re.MULTILINE), replacer, text)

def parse_include(line):
    """Check if the specified line contains an !include directive"""

    pattern = "!include(?:\s+)(?:[\"']?([\w./\\\\]*)[\"']?)"

    match = re.match(pattern, line)

    if match:
        return match.group(1)

def replace_defines(text, defines):
    """Replace defines in the specified text."""

    for define in defines.items():
        text = text.replace('${%s}' % define[0], define[1])

    return text

def nsis_scanner(node, env, path):
    """The scanner"""

    content = replace_defines(node.get_contents(), env['NSIS_DEFINES'])

    result = []

    for line in uncomment(content).split('\n'):
        include = parse_include(line)

        if include:
            for p in (os.getcwd(), ) + path:
                f = os.path.join(str(p), include)
                if os.path.isfile(f):
                    include_file = env.File(f)
                    result.append(include_file)
                    result += nsis_scanner(include_file, env, path)
                    break

    return result

def nsis_emitter(source, target, env):
    """The emitter"""

    return (target, source)

def nsis_generator(source, target, env, for_signature):
    """The generator"""

    result = '%s %s %s "/XOutFile %s" -- %s' % \
            (
                env['NSIS'],
                ' '.join(env['NSIS_FLAGS']),
                ' '.join(['"/D%s=%s"' % (x[0], x[1]) for x in env['NSIS_DEFINES'].items()]),
                target[0],
                source[0]
            )

    return result

def generate(env):
    env.Append(NSIS = 'makensis')
    env.Append(NSIS_PATH = [])
    env.Append(NSIS_FLAGS = []);
    env.Append(NSIS_DEFINES = {});

    import SCons.Scanner

    env.Append(SCANNERS = SCons.Scanner.Scanner(
        function = nsis_scanner,
        skeys = ['.nsi', '.nsh'],
        path_function = lambda env, node, targets, arg: SCons.Scanner.FindPathDirs('NSIS_PATH')(env, node, targets, arg) + (os.path.join(os.path.dirname(env.WhereIs(env['NSIS'])), 'Include'), )
    ))

    import SCons.Builder

    nsis_builder = SCons.Builder.Builder(
        generator = nsis_generator,
        emitter = nsis_emitter,
        suffix = '.exe',
        src_suffix = '.nsi'
    )

    env.Append(BUILDERS = {'NSIS': nsis_builder})

def exists(env):
    return env.Detect(env['NSIS'])
