"""
FreeLAN types.
"""

from . import (
    native,
    ffi,
)
from .error import check_error_context


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
        @check_error_context
        def __init__(self, _str, ectx):
            """
            Create an instance from its string representation.

            :param _str: The string representation.
            """
            self._opaque_ptr = from_string(ectx, _str)

        def __del__(self):
            free(self._opaque_ptr)
            self._opaque_ptr = None

        @check_error_context
        def __str__(self, ectx):
            """
            Get the string representation of.

            :returns: The string representation.
            """
            value_ptr = to_string(ectx, self._opaque_ptr)
            value = ffi.string(value_ptr)
            native.freelan_free(value_ptr)

            return value

        def __repr__(self):
            return '{classname}({ptr})'.format(
                classname=self.__class__.__name__,
                ptr=self._opaque_ptr,
            )

    return Wrapper


class IPv4Address(NativeType('IPv4Address')):
    """
    An IPv4 address.
    """
