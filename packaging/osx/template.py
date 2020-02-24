"""A SCons builder for template files"""


def template_emitter(target, source, env):
    env.Depends(target, env['TEMPLATE_DICT'])

    return (target, source)


def template_action(target, source, env):
    _dict = env['TEMPLATE_DICT'].value

    template = source[0].get_contents()

    with open(target[0].abspath, 'wb') as targf:
        targf.write(
            template.decode().format(**_dict).encode()
        )


def generate(env):
    env.Append(TEMPLATE_DICT=env.Value({}))

    import SCons.Builder

    template_builder = SCons.Builder.Builder(
        action=template_action,
        emitter=template_emitter,
        src_suffix='.in',
    )

    env.Append(BUILDERS={'Template': template_builder})
