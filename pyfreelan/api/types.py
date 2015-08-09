"""
FreeLAN types.
"""

from functools import wraps

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
    def from_variant_typename(cls, typename, subtypes):
        if typename not in cls.wrapper_cache:
            cls.wrapper_cache[typename] = cls.create_variant_wrapper(
                typename,
                subtypes,
            )

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
        clone = getattr(native, 'freelan_%s_clone' % typename)
        less_than = getattr(native, 'freelan_%s_less_than' % typename)
        equal = getattr(native, 'freelan_%s_equal' % typename)

        class Wrapper(object):

            def __init__(self, opaque_ptr, owner=None):
                """
                Create an instance from its low-level opaque pointer.

                :param opaque_ptr: The opaque pointer to the underlying C
                instance.
                :param owner: An instance that owns the opaque_ptr. If set this
                instance won't attempt to destroy the opaque_ptr upon
                destruction.
                """
                if not isinstance(opaque_ptr, ffi.CData):
                    raise TypeError(
                        "opaque_ptr must be a low-level opaque pointer. Are "
                        "you missing a .from_string() call ?",
                    )

                self._opaque_ptr = opaque_ptr
                self._owner = owner

            @classmethod
            @check_error_context
            def from_string(cls, _str, ectx):
                """
                Create an instance from its string representation.

                :param _str: The string representation.
                """
                return cls(opaque_ptr=from_string(ectx, _str))

            def __del__(self):
                if getattr(self, '_owner', None):
                    del self._owner
                else:
                    if hasattr(self, '_opaque_ptr'):
                        free(self._opaque_ptr)
                        self._opaque_ptr = None

            @swallow_native_string
            @check_error_context
            def __str__(self, ectx):
                """
                Get the string representation.

                :returns: The string representation.
                """
                return to_string(ectx, self._opaque_ptr)

            @check_error_context
            def clone(self, ectx):
                """
                Clone an instance.

                :returns: A cloned instance.
                """
                return self.__class__(
                    opaque_ptr=clone(ectx, self._opaque_ptr),
                )

            def __eq__(self, other):
                if not isinstance(other, Wrapper):
                    return NotImplemented

                return bool(equal(self._opaque_ptr, other._opaque_ptr))

            def __ne__(self, other):
                result = self.__eq__(other)

                if result is NotImplemented:
                    return NotImplemented

                return not result

            def __lt__(self, other):
                if not isinstance(other, Wrapper):
                    return NotImplemented

                return bool(less_than(self._opaque_ptr, other._opaque_ptr))

            def __le__(self, other):
                lresult = self.__lt__(other)
                rresult = self.__eq__(other)

                if lresult is NotImplemented or rresult is NotImplemented:
                    return NotImplemented

                return lresult or rresult

            def __gt__(self, other):
                result = self.__le__(other)

                if result is NotImplemented:
                    return NotImplemented

                return not result

            def __ge__(self, other):
                result = self.__lt__(other)

                if result is NotImplemented:
                    return NotImplemented

                return not result

            def __repr__(self):
                return '{classname}({ptr})'.format(
                    classname=self.__class__.__name__,
                    ptr=self._opaque_ptr,
                )

            def __hash__(self):
                return hash(str(self))

        return Wrapper

    @classmethod
    def create_variant_wrapper(cls, typename, subtypes):
        """
        Create a API wrapper base class around the specified native variant
        type.

        :param typename: The name of the native type to wrap.
        :returns: An API wrapper class.
        """
        Wrapper = cls.from_typename(typename)

        def add_from_subtype(wrapper_cls, typename, subtype):
            from_subtype = getattr(
                native,
                'freelan_%s_from_%s' % (typename, subtype),
            )

            def func(cls, value):
                return cls(
                    opaque_ptr=from_subtype(
                        value._opaque_ptr,
                    ),
                )

            func.__name__ = 'from_%s' % subtype
            func.__doc__ = """
            Create an {typename} from an {subtype}.

            :param value: The {subtype} instance.
            :returns: A {typename} instance.
            """.format(
                typename=typename,
                subtype=subtype,
            )

            setattr(
                wrapper_cls,
                func.__name__,
                classmethod(func),
            )

        def add_as_subtype_ptr(wrapper_cls, typename, subtype):
            as_subtype = getattr(
                native,
                'freelan_%s_as_%s' % (typename, subtype),
            )

            def func(self):
                return as_subtype(self._opaque_ptr)

            func.__name__ = '_as_%s_ptr' % subtype
            func.__doc__ = """
            Get the {subtype} instance contained by the current {typename}
            instance.

            :returns: A {subtype} instance or `None` if the current instance
            does not contain a {subtype} instance.
            """.format(
                typename=typename,
                subtype=subtype,
            )

            setattr(
                wrapper_cls,
                func.__name__,
                func,
            )

        def add_as_subtype(wrapper_cls, typename, subtype, subtype_cls):
            as_subtype_ptr = getattr(
                wrapper_cls,
                '_as_%s_ptr' % subtype,
            )

            def func(self, ectx):
                opaque_ptr = as_subtype_ptr(self)

                if opaque_ptr != ffi.NULL:
                    return subtype_cls(
                        opaque_ptr=opaque_ptr,
                        owner=self,
                    )

            func.__name__ = 'as_%s' % subtype
            func.__doc__ = """
            Get the {subtype} instance contained in this {typename} instance.

            :returns: A {subtype} instance or `None` if this instance does not
            contain a {subtype} instance.
            """.format(
                typename=typename,
                subtype=subtype,
            )

            setattr(
                wrapper_cls,
                func.__name__,
                check_error_context(func),
            )

        subtypes_classes = {
            subtype: globals()[subtype]
            for subtype in subtypes
        }

        for subtype, subtype_cls in subtypes_classes.iteritems():
            add_from_subtype(Wrapper, typename, subtype)
            add_as_subtype_ptr(Wrapper, typename, subtype)
            add_as_subtype(Wrapper, typename, subtype, subtype_cls)

        def add_eq(wrapper_cls, typename, subtypes_classes):
            types_and_as_funcs = [
                (
                    subtype_cls,
                    getattr(
                        wrapper_cls,
                        'as_%s' % subtype,
                    ),
                )
                for subtype, subtype_cls in subtypes_classes.iteritems()
            ]
            original_func = wrapper_cls.__eq__

            def func(self, other):
                for subtype_cls, as_func in types_and_as_funcs:
                    if isinstance(other, subtype_cls):
                        return as_func(self) == other
                else:
                    return original_func(self, other)

            func.__name__ = '__eq__'

            setattr(
                wrapper_cls,
                func.__name__,
                func,
            )

        def add_lt(wrapper_cls, typename, subtypes_classes):
            types_and_as_funcs = [
                (
                    subtype_cls,
                    getattr(
                        wrapper_cls,
                        'as_%s' % subtype,
                    ),
                )
                for subtype, subtype_cls in subtypes_classes.iteritems()
            ]
            original_func = wrapper_cls.__lt__

            def func(self, other):
                for subtype_cls, as_func in types_and_as_funcs:
                    if isinstance(other, subtype_cls):
                        return as_func(self) < other
                else:
                    return original_func(self, other)

            func.__name__ = '__lt__'

            setattr(
                wrapper_cls,
                func.__name__,
                func,
            )

        add_eq(Wrapper, typename, subtypes_classes)
        add_lt(Wrapper, typename, subtypes_classes)

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
    def from_parts(cls, ip_address, port_number):
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
    def from_parts(cls, ip_address, port_number):
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
    def from_parts(cls, hostname, port_number):
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
    def from_parts(cls, ip_address, prefix_length, gateway=None):
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
    def from_parts(cls, ip_address, prefix_length, gateway=None):
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


class IPAddress(
    NativeType.from_variant_typename(
        'IPAddress',
        ['IPv4Address', 'IPv6Address'],
    ),
):
    """
    An IP address.
    """
