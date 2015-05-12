"""
FreeLAN API.
"""

import cffi

ffi = cffi.FFI()

ffi.cdef(
    """
    /* Misc */
    void* malloc(size_t size);
    void* realloc(void* ptr, size_t size);
    void free(void* ptr);

    /* Memory */
    void* freelan_malloc(size_t size);
    void* freelan_realloc(void* ptr, size_t size);
    void freelan_free(void* ptr);
    char* freelan_strdup(const char* str);
    void freelan_register_memory_functions(void* (*malloc_func)(size_t), void* (*realloc_func)(void*, size_t), void (*free_func)(void*), char* (*strdup_func)(const char*));

    /* Types */
    struct IPv4Address;
    struct IPv4Address* freelan_IPv4Address_from_string(const char* str);
    char* freelan_IPv4Address_to_string(struct IPv4Address* inst);
    void freelan_IPv4Address_free(struct IPv4Address* inst);
    """
)

native = ffi.verify(
    source='#include <freelan/freelan.h>',
    libraries=['freelan'],
    include_dirs=['./include'],
)

memory_map = {}
memory_usage = {
    'current': 0,
    'max': 0,
    'sum': 0,
}


@ffi.callback("void* (size_t)")
def malloc(size):
    result = native.malloc(size)
    memory_map[result] = size
    memory_usage['sum'] += size
    memory_usage['current'] += size
    memory_usage['max'] = max(memory_usage['max'], memory_usage['current'])

    return result


@ffi.callback("void* (void*, size_t)")
def realloc(ptr, size):
    result = native.realloc(ptr, size)

    if result != ffi.NULL:
        del memory_map[ptr]
        memory_map[result] = size
        memory_usage['sum'] += size
        memory_usage['current'] += size

    return result


@ffi.callback("void (void*)")
def free(ptr):
    result = native.free(ptr)
    memory_usage['current'] -= memory_map[ptr]
    del memory_map[ptr]

    return result


def register_memory_functions():
    """
    Instructs libfreelan to use the Python memory functions.

    Use only for debugging as it has a huge performance cost.
    """
    native.freelan_register_memory_functions(
        malloc,
        realloc,
        free,
        ffi.NULL,
    )


def unregister_memory_functions():
    """
    Instructs libfreelan to use the default memory functions.
    """
    native.freelan_register_memory_functions(
        ffi.NULL,
        ffi.NULL,
        ffi.NULL,
        ffi.NULL,
    )


def NativeType(typename):
    """
    Create a API wrapper base class around the specified native type.

    :param typename: The name of the native type to wrap.
    :returns: An API wrapper class.
    """
    from_string = getattr(native, 'freelan_%s_from_string' % typename)
    to_string = getattr(native, 'freelan_%s_to_string' % typename)
    free = getattr(native, 'freelan_%s_free' % typename)

    class Wrapper(object):
        def __init__(self, _str):
            """
            Create an instance from its string representation.

            :param _str: The string representation.
            """
            self._opaque_ptr = from_string(_str)

        def __del__(self):
            free(self._opaque_ptr)
            self._opaque_ptr = None

        def __str__(self):
            """
            Get the string representation of.

            :returns: The string representation.
            """
            value_ptr = to_string(self._opaque_ptr)
            value = ffi.string(value_ptr)
            native.freelan_free(value_ptr)

            return value

        def __repr__(self):
            return '{classname}({ptr})'.format(
                classname=self.__class__.__name__,
                ptr=self._opaque_ptr,
            )

    return Wrapper
