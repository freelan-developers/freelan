"""
FreeLAN API.
"""

import os
import platform

from distutils.ccompiler import (
    new_compiler,
    gen_preprocess_options,
)
from distutils.sysconfig import customize_compiler
from tempfile import gettempdir

import cffi


def extract_api():
    """
    Extract the API from the header files.
    """
    compiler = new_compiler()
    customize_compiler(compiler)

    if platform.system() == 'Windows':
        # Because life is unfair, preprocess() is not implemented in distutils
        # for Windows so we have to do it ourselves.
        def preprocess(source, output_file, extra_postargs):
            if not compiler.initialized:
                compiler.initialize()

            args = [compiler.cc, '/nologo']
            args.extend(extra_postargs)
            args.append('/Fi%s' % output_file)
            args.append(source)

            return compiler.spawn(args)

        compiler.preprocess = preprocess

    input_file = os.path.join(gettempdir(), 'freelan-api.c')
    output_file = os.path.join(gettempdir(), 'freelan-api.h')

    try:
        with open(input_file, 'w') as _file:
            _file.write("#include <freelan.h>\n")  # pragma: no branch

        compiler.preprocess(
            source=input_file,
            output_file=output_file,
            # Remove the line numbers from the output.
            extra_postargs=[
                '-P',
            ] + gen_preprocess_options(
                macros=[
                    # Disable unwanted includes in the API to only keep our
                    # defined methods.
                    ('FREELAN_API_NO_INCLUDES', 1),
                ],
                include_dirs=[],
            )
        )

        try:
            with open(output_file, 'r') as _file:
                return _file.read()

        finally:
            os.remove(output_file)

    finally:
        os.remove(input_file)


ffi = cffi.FFI()

# Needed for the memory inspection tool.
ffi.cdef("""
void* malloc(size_t size);
void* realloc(void* ptr, size_t size);
void free(void* ptr);
""")

ffi.cdef(extract_api())
ffi.set_source(
    "_pyfreelan",
    "#include <freelan.h>",
    libraries=['freelan'],
)

if __name__ == '__main__':
    ffi.compile()
