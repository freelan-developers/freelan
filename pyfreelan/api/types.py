"""
FreeLAN types.
"""

from functools import wraps

from . import native

from .error import (
    from_native_string,
    check_error_context,
)


def swallow_native_string(func):
    """
    Decorator that swallow native strings from function
    results and converts them in Python strings.

    :param func: The function to decorate.
    :returns: The decorated function.
    """
    @wraps(func)
    def wrapper(*args, **kwargs):
        return from_native_string(func(*args, **kwargs), free_on_success=True)

    wrapper.wrapped = func

    return wrapper


class NativeType(object):
    wrapper_cache = {}

    @classmethod
    def from_typename(cls, typename):
        if typename not in cls.wrapper_cache:
            cls.wrapper_cache[typename] = cls.create_wrapper(typename)

        return cls.wrapper_cache[typename]

    @classmethod
    def create_wrapper(cls, typename):
        """
        Create a API wrapper base class around the specified native type.

        :param typename: The name of the native type to wrap.
        :returns: An API wrapper class.
        """
        from_string = getattr(native, 'freelan_%s_from_string' % typename)
        to_string = getattr(native, 'freelan_%s_to_string' % typename)
        free = getattr(native, 'freelan_%s_free' % typename)
        less_than = getattr(native, 'freelan_%s_less_than' % typename)
        equal = getattr(native, 'freelan_%s_equal' % typename)

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

            @swallow_native_string
            @check_error_context
            def __str__(self, ectx):
                """
                Get the string representation of.

                :returns: The string representation.
                """
                return to_string(ectx, self._opaque_ptr)

            def __eq__(self, other):
                return equal(self._opaque_ptr, other._opaque_ptr)

            def __lt__(self, other):
                return less_than(self._opaque_ptr, other._opaque_ptr)

            def __repr__(self):
                return '{classname}({ptr})'.format(
                    classname=self.__class__.__name__,
                    ptr=self._opaque_ptr,
                )

        return Wrapper


class IPv4Address(NativeType.from_typename('IPv4Address')):
    """
    An IPv4 address.
    """


class IPv6Address(NativeType.from_typename('IPv6Address')):
    """
    An IPv6 address.
    """
