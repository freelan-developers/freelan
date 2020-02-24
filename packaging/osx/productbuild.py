"""A SCons builder for productbuild"""

from xml.dom import minidom

import SCons.Warnings
import SCons.Errors


def get_nodes(element, path):
    if not hasattr(element, 'nodeName'):
        return []

    separator = '/'

    if separator in path:
        tag, subpath = path.split(separator, 1)
    else:
        tag, subpath = path, None

    if tag == '':
        tag = '#document'

    if element.nodeName != tag:
        return []

    if subpath:
        result = []

        for node in element.childNodes:
            result.extend(get_nodes(node, subpath))

        return result

    return [element]


def productbuild_scanner(node, env, paths):
    result = []

    if node.exists():
        document = minidom.parseString(node.get_contents())
        xnodes = get_nodes(document, '/installer-gui-script/pkg-ref')
        packages = [xnode.childNodes[0].nodeValue for xnode in xnodes]

        for package in packages:
            for path in paths:
                package_file = env.Dir(path).File(package)

                if package_file.exists() or package_file.has_builder():
                    result.append(package_file)
                    break

    return result


def productbuild_emitter(target, source, env):
    """The emitter"""

    env.Depends(target, env['PRODUCTBUILD_OPTIONS'])
    env.Depends(target, env['PRODUCTBUILD_RESOURCES'])

    return (target, source)


def productbuild_generator(target, source, env, for_signature):
    """The generator"""

    options = env['PRODUCTBUILD_OPTIONS'].value
    options_str = ' '.join(
        '--%s %s' % (key, value)
        for key, value in options.items()
    )

    if env['PRODUCTBUILD_RESOURCES']:
        options_str = options_str + ' --resources $PRODUCTBUILD_RESOURCES'

    package_path = env['PRODUCTBUILD_PACKAGE_PATH']

    if package_path:
        options_str = options_str + ' ' + ' '.join(
            '--package-path %s' % path
            for path in package_path
        )

    return '{executable} {options_str} --distribution $SOURCE $TARGET'.format(
        executable=env['PRODUCTBUILD'],
        options_str=options_str,
    )


class ProductBuildNotFound(SCons.Warnings.Warning):
    pass


def detect(env):
    try:
        return env['PRODUCTBUILD']
    except KeyError:
        pass

    productbuild = env.WhereIs('productbuild')

    if productbuild:
        return productbuild

    raise SCons.Errors.StopError(
        ProductBuildNotFound,
        "Unable to find productbuild"
    )


def generate(env):
    env.Append(PRODUCTBUILD=detect(env))
    env.Append(PRODUCTBUILD_PACKAGE_PATH=[])
    env.Append(PRODUCTBUILD_RESOURCES=None)
    env.Append(PRODUCTBUILD_OPTIONS=env.Value({}))

    import SCons.Scanner

    env.Append(SCANNERS=SCons.Scanner.Scanner(
        function=productbuild_scanner,
        skeys=['.xml'],
        path_function=SCons.Scanner.FindPathDirs('PRODUCTBUILD_PACKAGE_PATH'),
    ))

    import SCons.Builder

    productbuild_builder = SCons.Builder.Builder(
        generator=productbuild_generator,
        emitter=productbuild_emitter,
        suffix='.pkg',
        src_suffix='.xml',
    )

    env.Append(BUILDERS={'ProductBuild': productbuild_builder})


def exists(env):
    return env.Detect(env['PRODUCTBUILD'])
