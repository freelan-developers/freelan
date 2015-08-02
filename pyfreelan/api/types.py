"""
FreeLAN types.
"""

from functools import (
    wraps,
    total_ordering,
)

from . import (
    native,
    ffi,
)

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

        @total_ordering
        class Wrapper(object):

            def __init__(self, opaque_ptr):
                """
                Create an instance from its low-level opaque pointer.

                :param opaque_ptr: The opaque pointer to the underlying C
                instance.
                """
                if not isinstance(opaque_ptr, ffi.CData):
                    raise TypeError(
                        "opaque_ptr must be a low-level opaque pointer. Are "
                        "you missing a .from_string() call ?",
                    )

                self._opaque_ptr = opaque_ptr

            @classmethod
            @check_error_context
            def from_string(cls, _str, ectx):
                """
                Create an instance from its string representation.

                :param _str: The string representation.
                """
                return cls(opaque_ptr=from_string(ectx, _str))

            def __del__(self):
                if hasattr(self, '_opaque_ptr'):
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
                if not isinstance(other, Wrapper):
                    return NotImplemented

                return equal(self._opaque_ptr, other._opaque_ptr)

            def __lt__(self, other):
                if not isinstance(other, Wrapper):
                    return NotImplemented

                return less_than(self._opaque_ptr, other._opaque_ptr)

            def __repr__(self):
                return '{classname}({ptr})'.format(
                    classname=self.__class__.__name__,
                    ptr=self._opaque_ptr,
                )

            def __hash__(self):
                return hash(str(self))

        return Wrapper


class IPv4Address(NativeType.from_typename('IPv4Address')):
    """
    An IPv4 address.
    """


class IPv6Address(NativeType.from_typename('IPv6Address')):
    """
    An IPv6 address.
    """


class Hostname(NativeType.from_typename('Hostname')):
    """
    A hostname.
    """


class PortNumber(NativeType.from_typename('PortNumber')):
    """
    A port number.
    """
    @classmethod
    def from_integer(cls, value):
        return cls.from_string(str(value))

    def __int__(self):
        return int(str(self))

    def __hash__(self):
        return int(self)


class IPv4PrefixLength(NativeType.from_typename('IPv4PrefixLength')):
    """
    A IPv4 prefix length.
    """
    @classmethod
    def from_integer(cls, value):
        return cls.from_string(str(value))

    def __int__(self):
        return int(str(self))

    def __hash__(self):
        return int(self)


class IPv6PrefixLength(NativeType.from_typename('IPv6PrefixLength')):
    """
    A IPv6 prefix length.
    """
    @classmethod
    def from_integer(cls, value):
        return cls.from_string(str(value))

    def __int__(self):
        return int(str(self))

    def __hash__(self):
        return int(self)


class IPv4Endpoint(NativeType.from_typename('IPv4Endpoint')):
    """
    An IPv4 endpoint.
    """

    @classmethod
    def from_parts(self, ip_address, port_number):
        """
        Create an IPv4Endpoint from its parts.

        :param ip_address: The IPv4Address component.
        :param port_number: The PortNumber component.
        :returns: An IPv4Endpoint instance.
        """
        return IPv4Endpoint(
            opaque_ptr=native.freelan_IPv4Endpoint_from_parts(
                ip_address._opaque_ptr,
                port_number._opaque_ptr,
            ),
        )

    @property
    def ip_address(self):
        return IPv4Address(
            opaque_ptr=native.freelan_IPv4Endpoint_get_IPv4Address(
                self._opaque_ptr,
            ),
        )

    @property
    def port_number(self):
        return PortNumber(
            opaque_ptr=native.freelan_IPv4Endpoint_get_PortNumber(
                self._opaque_ptr,
            ),
        )


class IPv6Endpoint(NativeType.from_typename('IPv6Endpoint')):
    """
    An IPv6 endpoint.
    """

    @classmethod
    def from_parts(self, ip_address, port_number):
        """
        Create an IPv4Endpoint from its parts.

        :param ip_address: The IPv6Address component.
        :param port_number: The PortNumber component.
        :returns: An IPv6Endpoint instance.
        """
        return IPv6Endpoint(
            opaque_ptr=native.freelan_IPv6Endpoint_from_parts(
                ip_address._opaque_ptr,
                port_number._opaque_ptr,
            ),
        )

    @property
    def ip_address(self):
        return IPv6Address(
            opaque_ptr=native.freelan_IPv6Endpoint_get_IPv6Address(
                self._opaque_ptr,
            ),
        )

    @property
    def port_number(self):
        return PortNumber(
            opaque_ptr=native.freelan_IPv6Endpoint_get_PortNumber(
                self._opaque_ptr,
            ),
        )


class HostnameEndpoint(NativeType.from_typename('HostnameEndpoint')):
    """
    A hostname endpoint.
    """

    @classmethod
    def from_parts(self, hostname, port_number):
        """
        Create a HostnameEndpoint from its parts.

        :param hostname: The Hostname component.
        :param port_number: The PortNumber component.
        :returns: A HostnameEndpoint instance.
        """
        return HostnameEndpoint(
            opaque_ptr=native.freelan_HostnameEndpoint_from_parts(
                hostname._opaque_ptr,
                port_number._opaque_ptr,
            ),
        )

    @property
    def hostname(self):
        return Hostname(
            opaque_ptr=native.freelan_HostnameEndpoint_get_Hostname(
                self._opaque_ptr,
            ),
        )

    @property
    def port_number(self):
        return PortNumber(
            opaque_ptr=native.freelan_HostnameEndpoint_get_PortNumber(
                self._opaque_ptr,
            ),
        )


class IPv4Route(NativeType.from_typename('IPv4Route')):
    """
    An IPv4 route.
    """

    @classmethod
    def from_parts(self, ip_address, prefix_length, gateway=None):
        """
        Create an IPv4Route from its parts.

        :param ip_address: The IPv4Address component.
        :param prefix_length: The IPv4PrefixLength component.
        :param gateway: The gateway component (optional).
        :returns: An IPv4Route instance.
        """
        return IPv4Route(
            opaque_ptr=native.freelan_IPv4Route_from_parts(
                ip_address._opaque_ptr,
                prefix_length._opaque_ptr,
                gateway._opaque_ptr if gateway else ffi.NULL,
            ),
        )

    @property
    def ip_address(self):
        return IPv4Address(
            opaque_ptr=native.freelan_IPv4Route_get_IPv4Address(
                self._opaque_ptr,
            ),
        )

    @property
    def prefix_length(self):
        return IPv4PrefixLength(
            opaque_ptr=native.freelan_IPv4Route_get_IPv4PrefixLength(
                self._opaque_ptr,
            ),
        )

    @property
    def gateway(self):
        opaque_ptr = native.freelan_IPv4Route_get_IPv4Address_gateway(
            self._opaque_ptr,
        )

        if opaque_ptr != ffi.NULL:
            return IPv4Address(opaque_ptr=opaque_ptr)


class IPv6Route(NativeType.from_typename('IPv6Route')):
    """
    An IPv6 route.
    """

    @classmethod
    def from_parts(self, ip_address, prefix_length, gateway=None):
        """
        Create an IPv6Route from its parts.

        :param ip_address: The IPv6Address component.
        :param prefix_length: The IPv6PrefixLength component.
        :param gateway: The gateway component (optional).
        :returns: An IPv6Route instance.
        """
        return IPv6Route(
            opaque_ptr=native.freelan_IPv6Route_from_parts(
                ip_address._opaque_ptr,
                prefix_length._opaque_ptr,
                gateway._opaque_ptr if gateway else ffi.NULL,
            ),
        )

    @property
    def ip_address(self):
        return IPv6Address(
            opaque_ptr=native.freelan_IPv6Route_get_IPv6Address(
                self._opaque_ptr,
            ),
        )

    @property
    def prefix_length(self):
        return IPv6PrefixLength(
            opaque_ptr=native.freelan_IPv6Route_get_IPv6PrefixLength(
                self._opaque_ptr,
            ),
        )

    @property
    def gateway(self):
        opaque_ptr = native.freelan_IPv6Route_get_IPv6Address_gateway(
            self._opaque_ptr,
        )

        if opaque_ptr != ffi.NULL:
            return IPv6Address(opaque_ptr=opaque_ptr)
