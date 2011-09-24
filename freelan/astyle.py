"""An A-Style builder for SCons"""

def astyle_emitter(source, target, env):
    """The emitter"""

    return (target, source)

def generate(env):
    env.Append(ASTYLE_FLAGS = ['-b', '-t', '-C', '-S', '-K', '-N', '-w', '-n']);

    import SCons.Builder

    astyle_builder = SCons.Builder.Builder(
        action = "astyle ${ASTYLE_FLAGS} ${SOURCES}",
        emitter = astyle_emitter
    )

    env.Append(BUILDERS = { 'AStyle': astyle_builder, })

def exists(env):
    return env.Detect("astyle")
