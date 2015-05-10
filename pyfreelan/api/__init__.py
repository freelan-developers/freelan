"""
FreeLAN API.
"""

import os
import cffi

ffi = cffi.FFI()

ffi.cdef(
"""
struct IPv4Address;
struct IPv4Address* freelan_IPv4Address_from_string(const char* str);
char* freelan_IPv4Address_to_string(struct IPv4Address* inst);
"""
)

native = ffi.verify('', libraries=['freelan'])


def api_wrapper(typename):
    """
    Create a API wrapper base class around the specified type.

    :param typename: The name of the native type to wrap.
    :returns: An API wrapper class.
    """
    from_string = getattr(native, 'freelan_%s_from_string' % typename)
    to_string = getattr(native, 'freelan_%s_to_string' % typename)

    class Wrapper(object):
        def __init__(self, _str):
            """
            Create an instance from its string representation.

            :param _str: The string representation.
            """
            self._opaque_ptr = from_string(_str)

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
