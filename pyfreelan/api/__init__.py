"""
FreeLAN API.
"""

import cffi

ffi = cffi.FFI()

ffi.cdef(
    """
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


def api_wrapper(typename):
    """
    Create a API wrapper base class around the specified type.

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
            return ffi.string(to_string(self._opaque_ptr))

        def __repr__(self):
            return '{classname}({ptr})'.format(
                classname=self.__class__.__name__,
                ptr=self._opaque_ptr,
            )

    return Wrapper
