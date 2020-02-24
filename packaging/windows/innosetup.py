"""A INNO Setup builder for SCons"""

import re
import os

import SCons.Warnings
import SCons.Errors


def uncomment(text):
    """Remove all INNO Setup comments from the specified text"""

    def replacer(match):
        s = match.group(0)
        if s.startswith(';'):
            return ''
        else:
            return s

    pattern = ';.*?$|"[^"]*"|\'[^"]*\''

    text = str(text)
    return re.sub(re.compile(pattern, re.DOTALL | re.MULTILINE), replacer, text)


def parse_define(line):
    """Check if the specified line contains an #define directive"""

    pattern = r'#(?:\s)*define\s*([\w_]+)(?:\s)*["\']?(.*)["\']'

    line = str(line)
    match = re.match(pattern, line, re.IGNORECASE)

    if match:
        return (match.group(1), match.group(2))


def replace_defines(text, defines):
    """Replace defines in the specified text."""

    defines = defines.copy()
    lines = text.split('\n')
    result = []

    for line in lines:
        define = parse_define(line)

        if (define):
            defines.setdefault(define[0], define[1])

        for define in defines.items():
            line = line.replace('{#%s}' % define[0], define[1])

        result.append(line)

    return '\n'.join(result)


def get_config(source, env):
    """Get a configuration from the specified source."""

    import configparser
    from io import StringIO

    config = configparser.ConfigParser(strict=False)
    config.readfp(
        StringIO(replace_defines(source.get_contents().decode(), env['ISCC_DEFINES'])))

    return config


def parse_file(line):
    """Check if the specified line contains an Source: directive"""

    pattern = r'Source(?:\s)*:\s*([^;]+)(?:\s)*(?:;|$)'

    match = re.match(pattern, line, re.IGNORECASE)

    if match:
        return match.group(1)


def get_sections(lines):
    """Get all the sections and the lines they contain."""

    result = {}
    current_section = None

    for line in lines:
        m = re.match(r'\[(\w+)\]', line)
        if m:
            current_section = m.group(1)
            result[current_section] = []
        elif current_section:
            result[current_section].append(line)

    return result


def get_files(source, env):
    """Get the list of referenced files from the specified source."""

    sections = get_sections(
        replace_defines(uncomment(source.get_contents()), env['ISCC_DEFINES']).split('\n'))

    result = []

    for line in sections.get('Files', []):
        f = parse_file(line)
        if f:
            result.append(env.File(f))

    config = get_config(source, env)

    for images in {'WizardSmallImageFile', 'WizardImageFile'}:
        image_file = config.get('Setup', 'WizardSmallImageFile')
        result.append(env.File(image_file))

    return result


def innosetup_scanner(node, env, path):
    """The scanner"""

    return get_files(node, env)


def innosetup_emitter(target, source, env):
    """The emitter"""

    config = get_config(source[0], env)

    output_dir = config.get('Setup', 'OutputDir')
    output_base_filename = config.get('Setup', 'OutputBaseFilename')

    if output_dir and output_base_filename:
        target = os.path.join(output_dir, output_base_filename + '.exe')

    env.Depends(target, env['ISCC'])

    return (target, source)


def innosetup_generator(target, source, env, for_signature):
    """The generator"""

    def to_define_option(item):
        if item[1]:
            return '"/d%s=%s"' % (item[0], item[1])
        else:
            return '"/d%s"' % item[0]

    result = '"%s" %s %s %s "/o%s" "/f%s" %s' % \
        (
            env['ISCC'],
            ' '.join(env['ISCC_FLAGS']),
            ' '.join('"/i%s"' % x for x in env['ISCC_PATH']),
            ' '.join([to_define_option(x)
                      for x in env['ISCC_DEFINES'].items()]),
            os.path.dirname(str(target[0])),
            os.path.splitext(os.path.basename(str(target[0])))[0],
            source[0]
        )

    return result


class ISCCNotFound(SCons.Warnings.Warning):
    pass


def detect(env):
    try:
        return env['ISCC']
    except KeyError:
        pass

    iscc = env.WhereIs('iscc.exe')

    if iscc:
        return iscc

    paths = [
        os.path.join(os.environ.get('PROGRAMFILES', ''), 'Inno Setup 6'),
        os.path.join(os.environ.get('PROGRAMFILES(X86)', ''), 'Inno Setup 6'),
    ]

    for path in paths:
        iscc = os.path.join(path, 'iscc.exe')

        if os.path.exists(iscc):
            return iscc

    raise SCons.Errors.StopError(
        ISCCNotFound,
        "Unable to find iscc.exe"
    )


def generate(env):
    env.Append(ISCC=detect(env))
    env.Append(ISCC_PATH=[])
    env.Append(ISCC_FLAGS=[])
    env.Append(ISCC_DEFINES={})

    import SCons.Scanner

    env.Append(SCANNERS=SCons.Scanner.Scanner(
        function=innosetup_scanner,
        skeys=['.iss'],
        path_function=SCons.Scanner.FindPathDirs('ISCC_PATH')
    ))

    import SCons.Builder

    innosetup_builder = SCons.Builder.Builder(
        generator=innosetup_generator,
        emitter=innosetup_emitter,
        suffix='.exe',
        src_suffix='.iss'
    )

    env.Append(BUILDERS={'INNOSetup': innosetup_builder})


def exists(env):
    return env.Detect(env['ISCC'])
