"""A SCons builder for plist files"""


def generate_script_emitter(target, source, env):
    env.Depends(target, env.Value(env['GENERATE_SCRIPT_TEMPLATE']))

    return (target, source)


def generate_script_action(target, source, env):
    template = env['GENERATE_SCRIPT_TEMPLATE']

    for targ in target:
        with open(targ.abspath, 'w') as targf:
            targf.write(
                template.format(
                    commands='\n'.join(source[0].value),
                ),
            )


def generate(env):
    env.Append(GENERATE_SCRIPT_TEMPLATE="""#!/bin/sh

{commands}
""")

    import SCons.Builder

    generate_script_builder = SCons.Builder.Builder(
        action=generate_script_action,
        emitter=generate_script_emitter,
        suffix='.sh',
    )

    env.Append(BUILDERS={'GenerateScript': generate_script_builder})
