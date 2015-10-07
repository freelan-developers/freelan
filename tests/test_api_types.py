"""
Types API tests.
"""

from __future__ import unicode_literals

import six

from unittest import TestCase
from functools import wraps
from contextlib import contextmanager

from mock import (
    patch,
    MagicMock,
)

from pyfreelan.api import ffi
from pyfreelan.api.types import (
    swallow_native_string,
    NativeType,
    EthernetAddress,
    IPv4Address,
    IPv6Address,
    Hostname,
    IPv4PrefixLength,
    IPv6PrefixLength,
    PortNumber,
    IPv4Endpoint,
    IPv6Endpoint,
    HostnameEndpoint,
    IPv4Route,
    IPv6Route,
    IPAddress,
    IPRoute,
    Host,
    IPEndpoint,
    HostEndpoint,
)
from pyfreelan.api.error import ErrorContext


def disable_error_context_checks(func):
    """
    Mocks out the error context checks.

    :param func: The function that must run without error context checks.
    :returns: A decorated function that runs withouts error context checks.
    """
    ectx = MagicMock(spec=ErrorContext)

    @contextmanager
    def passthrough(name, kwargs):
        kwargs = kwargs.copy()
        kwargs[name] = ectx

        yield kwargs

    @wraps(func)
    def wrapper(*args, **kwargs):
        with patch(
            'pyfreelan.api.error.inject_error_context',
            passthrough,
        ):
            kwargs['ectx'] = ectx
            return func(*args, **kwargs)

    return wrapper


class SwallowNativeStringTests(TestCase):

    def test_from_native_string_is_called(self):
        value = MagicMock()

        @swallow_native_string
        def func(*args, **kwargs):
            return value

        with patch(
            'pyfreelan.api.types.from_native_string',
        ) as from_native_string_mock:
            func()

        from_native_string_mock.assert_called_once_with(
            value,
            free_on_success=True,
        )

    def test_all_arguments_pass_through(self):
        value = MagicMock()

        @swallow_native_string
        def func(*args, **kwargs):
            self.assertEqual((42,), args)
            self.assertEqual({'foo': 'bar'}, kwargs)
            return value

        with patch(
            'pyfreelan.api.types.from_native_string',
        ) as from_native_string_mock:
            result = func(42, foo='bar')

        self.assertEqual(from_native_string_mock(value), result)

    def test_wrapped_function_is_accessible(self):
        def myfunc():
            pass

        decorated = swallow_native_string(myfunc)
        self.assertEqual(decorated.wrapped, myfunc)


class NativeTypeTests(TestCase):

    def setUp(self):
        self.native_patch = patch('pyfreelan.api.types.native')
        self.native = self.native_patch.start()

    def tearDown(self):
        self.native_patch.stop()

    def test_from_typename_creates_non_existing_wrappers(self):
        klass = MagicMock(spec=NativeType)
        klass.wrapper_cache = {}

        result = NativeType.from_typename.__func__(klass, 'foo')

        klass.create_wrapper.assert_called_once_with('foo')
        self.assertEqual(
            {'foo': klass.create_wrapper('foo')},
            klass.wrapper_cache,
        )
        self.assertEqual(klass.create_wrapper('foo'), result)

    def test_from_typename_returns_existing_wrappers(self):
        klass = MagicMock(spec=NativeType)
        wrapper = MagicMock()
        klass.wrapper_cache = {'foo': wrapper}

        result = NativeType.from_typename.__func__(klass, 'foo')

        self.assertEqual([], klass.create_wrapper.mock_calls)
        self.assertEqual({'foo': wrapper}, klass.wrapper_cache)
        self.assertEqual(wrapper, result)

    def test_from_variant_typename_creates_non_existing_wrappers(self):
        klass = MagicMock(spec=NativeType)
        klass.wrapper_cache = {}

        result = NativeType.from_variant_typename.__func__(
            klass,
            'foo',
            ['bar'],
        )

        klass.create_variant_wrapper.assert_called_once_with('foo', ['bar'])
        self.assertEqual(
            {'foo': klass.create_variant_wrapper('foo', ['bar'])},
            klass.wrapper_cache,
        )
        self.assertEqual(klass.create_variant_wrapper('foo', ['bar']), result)

    def test_from_variant_typename_returns_existing_wrappers(self):
        klass = MagicMock(spec=NativeType)
        wrapper = MagicMock()
        klass.wrapper_cache = {'foo': wrapper}

        result = NativeType.from_variant_typename.__func__(
            klass,
            'foo',
            ['bar'],
        )

        self.assertEqual([], klass.create_variant_wrapper.mock_calls)
        self.assertEqual({'foo': wrapper}, klass.wrapper_cache)
        self.assertEqual(wrapper, result)

    def test_wrapper_init_stores_pointer(self):
        wrapper = NativeType.create_wrapper('foo')
        instance = MagicMock(spec=wrapper)
        opaque_ptr = MagicMock(spec=ffi.CData)

        wrapper.__init__(instance, opaque_ptr)

        self.assertEqual(opaque_ptr, instance._opaque_ptr)

    def test_wrapper_init_checks_pointer_type(self):
        wrapper = NativeType.create_wrapper('foo')

        with self.assertRaises(TypeError):
            wrapper("bar")

    @disable_error_context_checks
    def test_wrapper_from_string(self, ectx):
        wrapper = NativeType.create_wrapper('foo')
        from_string = self.native.freelan_foo_from_string
        from_string.return_value = MagicMock(spec=ffi.CData)

        instance = wrapper.from_string("mystr")

        from_string.assert_called_once_with(ectx, b"mystr")
        self.assertEqual(from_string("mystr"), instance._opaque_ptr)

    def test_wrapper_del_calls_free(self):
        wrapper = NativeType.create_wrapper('foo')
        instance = MagicMock(spec=wrapper)
        native_ptr = MagicMock()
        instance._opaque_ptr = native_ptr
        free = self.native.freelan_foo_free

        wrapper.__del__(instance)

        free.assert_called_once_with(native_ptr)
        self.assertEqual(None, instance._opaque_ptr)

    @disable_error_context_checks
    def test_wrapper_str_calls_to_string(self, ectx):
        wrapper = NativeType.create_wrapper('foo')
        instance = MagicMock(spec=wrapper)
        native_ptr = MagicMock()
        instance._opaque_ptr = native_ptr
        to_string = self.native.freelan_foo_to_string

        result = wrapper.__str__.wrapped(instance)

        to_string.assert_called_once_with(ectx, native_ptr)
        self.assertEqual(to_string(ectx, native_ptr), result)

    @disable_error_context_checks
    def test_wrapper_clone(self, ectx):
        wrapper = NativeType.create_wrapper('foo')
        instance = MagicMock()
        native_ptr = MagicMock()
        instance.__class__ = MagicMock()
        instance._opaque_ptr = native_ptr
        clone = self.native.freelan_foo_clone

        result = wrapper.clone.wrapped(instance, ectx)

        clone.assert_called_once_with(ectx, native_ptr)
        self.assertEqual(
            instance.__class__(opaque_ptr=clone(ectx, native_ptr)),
            result,
        )

    def test_wrapper_eq_calls_equal(self):
        wrapper = NativeType.create_wrapper('foo')
        instance_a = MagicMock(spec=wrapper)
        instance_b = MagicMock(spec=wrapper)
        native_ptr_a = MagicMock()
        native_ptr_b = MagicMock()
        instance_a._opaque_ptr = native_ptr_a
        instance_b._opaque_ptr = native_ptr_b
        equal = self.native.freelan_foo_equal

        result = wrapper.__eq__(instance_a, instance_b)

        equal.assert_called_once_with(native_ptr_a, native_ptr_b)
        self.assertEqual(bool(equal(native_ptr_a, native_ptr_b)), result)

    def test_wrapper_lt_calls_less_than(self):
        wrapper = NativeType.create_wrapper('foo')
        instance_a = MagicMock(spec=wrapper)
        instance_b = MagicMock(spec=wrapper)
        native_ptr_a = MagicMock()
        native_ptr_b = MagicMock()
        instance_a._opaque_ptr = native_ptr_a
        instance_b._opaque_ptr = native_ptr_b
        less_than = self.native.freelan_foo_less_than

        result = wrapper.__lt__(instance_a, instance_b)

        less_than.assert_called_once_with(native_ptr_a, native_ptr_b)
        self.assertEqual(bool(less_than(native_ptr_a, native_ptr_b)), result)

    def test_wrapper_repr(self):
        wrapper = NativeType.create_wrapper('foo')
        instance = MagicMock(spec=wrapper)
        instance.__class__.__name__ = str('MyClass')
        native_ptr = '0x12345678'
        instance._opaque_ptr = native_ptr

        result = wrapper.__repr__(instance)

        self.assertEqual('MyClass(0x12345678)', result)

    def test_wrapper_comparison(self):
        def from_string(_, s):
            result = MagicMock(spec=ffi.CData)
            result.value = s
            return result

        def equal(lhs, rhs):
            self.assertEqual({b"a", b"b"}, {lhs.value, rhs.value})
            return lhs.value == rhs.value

        def less_than(lhs, rhs):
            self.assertEqual({b"a", b"b"}, {lhs.value, rhs.value})
            return lhs.value < rhs.value

        self.native.freelan_foo_from_string = from_string
        self.native.freelan_foo_equal = equal
        self.native.freelan_foo_less_than = less_than

        wrapper = NativeType.create_wrapper('foo')
        instance_a = wrapper.from_string("a")
        instance_b = wrapper.from_string("b")

        self.assertFalse(instance_a == instance_b)
        self.assertTrue(instance_a != instance_b)
        self.assertFalse(instance_a > instance_b)
        self.assertFalse(instance_a >= instance_b)
        self.assertTrue(instance_a < instance_b)
        self.assertTrue(instance_a <= instance_b)

    def test_wrapper_invalid_comparison(self):
        wrapper = NativeType.create_wrapper('foo')
        from_string = self.native.freelan_foo_from_string
        from_string.return_value = MagicMock(spec=ffi.CData)
        instance = wrapper.from_string("instance")

        # Wrapper types can only be compared to instances of the same class.
        self.assertFalse(instance == "instance")

        if six.PY2:
            self.assertTrue(instance < "instance")
        else:
            with self.assertRaises(TypeError):
                instance < "instance"

    def test_wrapper_hash(self):
        wrapper = NativeType.create_wrapper('foo')
        instance = MagicMock(spec=wrapper)
        native_ptr = MagicMock()
        instance._opaque_ptr = native_ptr
        instance.__str__ = MagicMock(
            spec=instance.__str__,
            return_value="fooooo",
        )

        result = wrapper.__hash__(instance)

        instance.__str__.assert_called_once_with()
        self.assertEqual(hash(instance.__str__()), result)


class FinalTypesTests(TestCase):

    def test_EthernetAddress_clone(self):
        a = EthernetAddress.from_string("ab:cd:ef:12:34:56")
        b = a.clone()

        self.assertIsNot(a, b)
        self.assertNotEqual(a._opaque_ptr, b._opaque_ptr)
        self.assertEqual(a, b)

    def test_EthernetAddress(self):
        a = EthernetAddress.from_string("ab:cd:ef:12:34:56")
        b = EthernetAddress.from_string("ab:cd:ef:12:34:57")

        self.assertIsNot(a, b)
        self.assertNotEqual(hash(a), hash(b))
        self.assertNotEqual(a, b)
        self.assertLess(a, b)
        self.assertEqual(1, len({a, a}))
        self.assertEqual(2, len({a, b}))

    def test_IPv4Address_clone(self):
        a = IPv4Address.from_string("0.0.0.1")
        b = a.clone()

        self.assertIsNot(a, b)
        self.assertNotEqual(a._opaque_ptr, b._opaque_ptr)
        self.assertEqual(a, b)

    def test_IPv4Address(self):
        a = IPv4Address.from_string("0.0.0.1")
        b = IPv4Address.from_string("0.0.0.2")

        self.assertIsNot(a, b)
        self.assertNotEqual(hash(a), hash(b))
        self.assertNotEqual(a, b)
        self.assertLess(a, b)
        self.assertEqual(1, len({a, a}))
        self.assertEqual(2, len({a, b}))

    def test_IPv6Address(self):
        a = IPv6Address.from_string("ffe0::abcd")
        b = IPv6Address.from_string("ffe0::abce")

        self.assertIsNot(a, b)
        self.assertNotEqual(hash(a), hash(b))
        self.assertNotEqual(a, b)
        self.assertLess(a, b)
        self.assertEqual(1, len({a, a}))
        self.assertEqual(2, len({a, b}))

    def test_Hostname(self):
        a = Hostname.from_string("my.host.name1")
        b = Hostname.from_string("my.host.name2")

        self.assertIsNot(a, b)
        self.assertNotEqual(hash(a), hash(b))
        self.assertNotEqual(a, b)
        self.assertLess(a, b)
        self.assertEqual(1, len({a, a}))
        self.assertEqual(2, len({a, b}))

    def test_PortNumber(self):
        a = PortNumber.from_string("12000")
        b = PortNumber.from_string("12001")

        self.assertIsNot(a, b)
        self.assertNotEqual(hash(a), hash(b))
        self.assertNotEqual(a, b)
        self.assertLess(a, b)
        self.assertEqual(1, len({a, a}))
        self.assertEqual(2, len({a, b}))

    def test_PortNumber_from_integer(self):
        a = PortNumber.from_string("12000")
        b = PortNumber.from_integer(12000)

        self.assertEqual(a, b)

    def test_PortNumber_to_integer(self):
        instance = PortNumber.from_string("12000")

        self.assertEqual(12000, int(instance))

    def test_IPv4PrefixLength(self):
        a = IPv4PrefixLength.from_string("24")
        b = IPv4PrefixLength.from_string("25")

        self.assertIsNot(a, b)
        self.assertNotEqual(hash(a), hash(b))
        self.assertNotEqual(a, b)
        self.assertLess(a, b)
        self.assertEqual(1, len({a, a}))
        self.assertEqual(2, len({a, b}))

    def test_IPv4PrefixLength_from_integer(self):
        a = IPv4PrefixLength.from_string("24")
        b = IPv4PrefixLength.from_integer(24)

        self.assertEqual(a, b)

    def test_IPv4PrefixLength_to_integer(self):
        instance = IPv4PrefixLength.from_string("24")

        self.assertEqual(24, int(instance))

    def test_IPv6PrefixLength(self):
        a = IPv6PrefixLength.from_string("25")
        b = IPv6PrefixLength.from_string("26")

        self.assertIsNot(a, b)
        self.assertNotEqual(hash(a), hash(b))
        self.assertNotEqual(a, b)
        self.assertLess(a, b)
        self.assertEqual(1, len({a, a}))
        self.assertEqual(2, len({a, b}))

    def test_IPv6PrefixLength_from_integer(self):
        a = IPv6PrefixLength.from_string("26")
        b = IPv6PrefixLength.from_integer(26)

        self.assertEqual(a, b)

    def test_IPv6PrefixLength_to_integer(self):
        instance = IPv6PrefixLength.from_string("26")

        self.assertEqual(26, int(instance))

    def test_IPv4Endpoint(self):
        a = IPv4Endpoint.from_string("0.0.0.1:1234")
        b = IPv4Endpoint.from_string("0.0.0.2:1234")

        self.assertIsNot(a, b)
        self.assertNotEqual(hash(a), hash(b))
        self.assertNotEqual(a, b)
        self.assertLess(a, b)
        self.assertEqual(1, len({a, a}))
        self.assertEqual(2, len({a, b}))

    def test_IPv4Endpoint_parts(self):
        ip_address_str = "0.0.0.1"
        port_number_str = "1234"
        ip_address = IPv4Address.from_string(ip_address_str)
        port_number = PortNumber.from_string(port_number_str)

        a = IPv4Endpoint.from_string(
            "%s:%s" % (ip_address_str, port_number_str),
        )
        b = IPv4Endpoint.from_parts(ip_address, port_number)

        self.assertEqual(a, b)
        self.assertEqual(ip_address, b.ip_address)
        self.assertEqual(port_number, b.port_number)

    def test_IPv6Endpoint(self):
        a = IPv6Endpoint.from_string("[fe80::a:1]:1234")
        b = IPv6Endpoint.from_string("[fe80::a:1]:1235")

        self.assertIsNot(a, b)
        self.assertNotEqual(hash(a), hash(b))
        self.assertNotEqual(a, b)
        self.assertLess(a, b)
        self.assertEqual(1, len({a, a}))
        self.assertEqual(2, len({a, b}))

    def test_IPv6Endpoint_parts(self):
        ip_address_str = "fe80::a:1"
        port_number_str = "1234"
        ip_address = IPv6Address.from_string(ip_address_str)
        port_number = PortNumber.from_string(port_number_str)

        a = IPv6Endpoint.from_string(
            "[%s]:%s" % (ip_address_str, port_number_str),
        )
        b = IPv6Endpoint.from_parts(ip_address, port_number)

        self.assertEqual(a, b)
        self.assertEqual(ip_address, b.ip_address)
        self.assertEqual(port_number, b.port_number)

    def test_HostnameEndpoint(self):
        a = HostnameEndpoint.from_string("foo.bar:1234")
        b = HostnameEndpoint.from_string("foo.bar:1235")

        self.assertIsNot(a, b)
        self.assertNotEqual(hash(a), hash(b))
        self.assertNotEqual(a, b)
        self.assertLess(a, b)
        self.assertEqual(1, len({a, a}))
        self.assertEqual(2, len({a, b}))

    def test_HostnameEndpoint_parts(self):
        hostname_str = "foo.bar"
        port_number_str = "1234"
        hostname = Hostname.from_string(hostname_str)
        port_number = PortNumber.from_string(port_number_str)

        a = HostnameEndpoint.from_string(
            "%s:%s" % (hostname_str, port_number_str),
        )
        b = HostnameEndpoint.from_parts(hostname, port_number)

        self.assertEqual(a, b)
        self.assertEqual(hostname, b.hostname)
        self.assertEqual(port_number, b.port_number)

    def test_IPv4Route(self):
        a = IPv4Route.from_string("9.0.0.1/24")
        b = IPv4Route.from_string("9.0.1.1/24")

        self.assertIsNot(a, b)
        self.assertNotEqual(hash(a), hash(b))
        self.assertNotEqual(a, b)
        self.assertLess(a, b)
        self.assertEqual(1, len({a, a}))
        self.assertEqual(2, len({a, b}))

    def test_IPv4Route_parts(self):
        ip_address_str = "9.0.1.0"
        prefix_length_str = "24"
        gateway_str = "9.0.1.254"
        ip_address = IPv4Address.from_string(ip_address_str)
        prefix_length = IPv4PrefixLength.from_string(prefix_length_str)
        gateway = IPv4Address.from_string(gateway_str)

        a = IPv4Route.from_string(
            "%s/%s@%s" % (ip_address_str, prefix_length_str, gateway_str),
        )
        b = IPv4Route.from_parts(ip_address, prefix_length, gateway)

        self.assertEqual(a, b)
        self.assertEqual(ip_address, b.ip_address)
        self.assertEqual(prefix_length, b.prefix_length)
        self.assertEqual(gateway, b.gateway)

    def test_IPv4Route_no_gateway(self):
        ip_route = IPv4Route.from_string("9.0.0.1/24")

        self.assertIsNone(ip_route.gateway)

    def test_IPv6Route(self):
        a = IPv6Route.from_string("fe80::a:1/120")
        b = IPv6Route.from_string("fe80::b:1/120")

        self.assertIsNot(a, b)
        self.assertNotEqual(hash(a), hash(b))
        self.assertNotEqual(a, b)
        self.assertLess(a, b)
        self.assertEqual(1, len({a, a}))
        self.assertEqual(2, len({a, b}))

    def test_IPv6Route_parts(self):
        ip_address_str = "fe80::a:0"
        prefix_length_str = "120"
        gateway_str = "fe80::a:ffff"
        ip_address = IPv6Address.from_string(ip_address_str)
        prefix_length = IPv6PrefixLength.from_string(prefix_length_str)
        gateway = IPv6Address.from_string(gateway_str)

        a = IPv6Route.from_string(
            "%s/%s@%s" % (ip_address_str, prefix_length_str, gateway_str),
        )
        b = IPv6Route.from_parts(ip_address, prefix_length, gateway)

        self.assertEqual(a, b)
        self.assertEqual(ip_address, b.ip_address)
        self.assertEqual(prefix_length, b.prefix_length)
        self.assertEqual(gateway, b.gateway)

    def test_IPv6Route_no_gateway(self):
        ip_route = IPv6Route.from_string("fe80::a:0/24")

        self.assertIsNone(ip_route.gateway)

    def test_IPAddress(self):
        a = IPAddress.from_string("0.0.0.1")
        b = IPAddress.from_string("fe80::a:10")

        self.assertIsNot(a, b)

        self.assertFalse(a == b)
        self.assertTrue(a != b)
        self.assertTrue(a < b)
        self.assertTrue(a <= b)
        self.assertFalse(a > b)
        self.assertFalse(a >= b)

        self.assertNotEqual(hash(a), hash(b))
        self.assertEqual(1, len({a, a}))
        self.assertEqual(2, len({a, b}))

    def test_IPAddress_from_IPv4Address(self):
        value = IPv4Address.from_string("0.0.0.1")
        instance = IPAddress.from_IPv4Address(value)

        self.assertEqual(value, instance)
        self.assertEqual(value, instance.as_IPv4Address())
        self.assertIsNone(instance.as_IPv6Address())

        self.assertTrue(value == instance)
        self.assertFalse(value != instance)
        self.assertFalse(value < instance)
        self.assertTrue(value <= instance)
        self.assertFalse(value > instance)
        self.assertTrue(value >= instance)

    def test_IPAddress_from_IPv6Address(self):
        value = IPv6Address.from_string("fe80::a:1")
        instance = IPAddress.from_IPv6Address(value)

        self.assertEqual(value, instance)
        self.assertIsNone(instance.as_IPv4Address())
        self.assertEqual(value, instance.as_IPv6Address())

        self.assertTrue(value == instance)
        self.assertFalse(value != instance)
        self.assertFalse(value < instance)
        self.assertTrue(value <= instance)
        self.assertFalse(value > instance)
        self.assertTrue(value >= instance)

    def test_IPRoute(self):
        a = IPRoute.from_string("9.0.0.0/24")
        b = IPRoute.from_string("fe80::a:10:0/120")

        self.assertIsNot(a, b)

        self.assertFalse(a == b)
        self.assertTrue(a != b)
        self.assertTrue(a < b)
        self.assertTrue(a <= b)
        self.assertFalse(a > b)
        self.assertFalse(a >= b)

        self.assertNotEqual(hash(a), hash(b))
        self.assertEqual(1, len({a, a}))
        self.assertEqual(2, len({a, b}))

    def test_IPRoute_from_IPv4Route(self):
        value = IPv4Route.from_string("9.0.0.0/24")
        instance = IPRoute.from_IPv4Route(value)

        self.assertEqual(value, instance)
        self.assertEqual(value, instance.as_IPv4Route())
        self.assertIsNone(instance.as_IPv6Route())

        self.assertTrue(value == instance)
        self.assertFalse(value != instance)
        self.assertFalse(value < instance)
        self.assertTrue(value <= instance)
        self.assertFalse(value > instance)
        self.assertTrue(value >= instance)

    def test_IPRoute_from_IPv6Route(self):
        value = IPv6Route.from_string("fe80::a:1:0/120")
        instance = IPRoute.from_IPv6Route(value)

        self.assertEqual(value, instance)
        self.assertIsNone(instance.as_IPv4Route())
        self.assertEqual(value, instance.as_IPv6Route())

        self.assertTrue(value == instance)
        self.assertFalse(value != instance)
        self.assertFalse(value < instance)
        self.assertTrue(value <= instance)
        self.assertFalse(value > instance)
        self.assertTrue(value >= instance)

    def test_Host(self):
        a = Host.from_string("0.0.0.1")
        b = Host.from_string("fe80::a:10")

        self.assertIsNot(a, b)

        self.assertFalse(a == b)
        self.assertTrue(a != b)
        self.assertTrue(a < b)
        self.assertTrue(a <= b)
        self.assertFalse(a > b)
        self.assertFalse(a >= b)

        self.assertNotEqual(hash(a), hash(b))
        self.assertEqual(1, len({a, a}))
        self.assertEqual(2, len({a, b}))

    def test_Host_from_IPv4Address(self):
        value = IPv4Address.from_string("0.0.0.1")
        instance = Host.from_IPv4Address(value)

        self.assertEqual(value, instance)
        self.assertEqual(value, instance.as_IPv4Address())
        self.assertIsNone(instance.as_IPv6Address())
        self.assertIsNone(instance.as_Hostname())

        self.assertTrue(value == instance)
        self.assertFalse(value != instance)
        self.assertFalse(value < instance)
        self.assertTrue(value <= instance)
        self.assertFalse(value > instance)
        self.assertTrue(value >= instance)

    def test_Host_from_IPv6Address(self):
        value = IPv6Address.from_string("fe80::a:1")
        instance = Host.from_IPv6Address(value)

        self.assertEqual(value, instance)
        self.assertIsNone(instance.as_IPv4Address())
        self.assertEqual(value, instance.as_IPv6Address())
        self.assertIsNone(instance.as_Hostname())

        self.assertTrue(value == instance)
        self.assertFalse(value != instance)
        self.assertFalse(value < instance)
        self.assertTrue(value <= instance)
        self.assertFalse(value > instance)
        self.assertTrue(value >= instance)

    def test_Host_from_Hostname(self):
        value = Hostname.from_string("foo.bar.net")
        instance = Host.from_Hostname(value)

        self.assertEqual(value, instance)
        self.assertIsNone(instance.as_IPv4Address())
        self.assertIsNone(instance.as_IPv6Address())
        self.assertEqual(value, instance.as_Hostname())

        self.assertTrue(value == instance)
        self.assertFalse(value != instance)
        self.assertFalse(value < instance)
        self.assertTrue(value <= instance)
        self.assertFalse(value > instance)
        self.assertTrue(value >= instance)

    def test_IPEndpoint(self):
        a = IPEndpoint.from_string("0.0.0.1:12000")
        b = IPEndpoint.from_string("[fe80::a:10]:12000")

        self.assertIsNot(a, b)

        self.assertFalse(a == b)
        self.assertTrue(a != b)
        self.assertTrue(a < b)
        self.assertTrue(a <= b)
        self.assertFalse(a > b)
        self.assertFalse(a >= b)

        self.assertNotEqual(hash(a), hash(b))
        self.assertEqual(1, len({a, a}))
        self.assertEqual(2, len({a, b}))

    def test_IPEndpoint_from_IPv4Endpoint(self):
        value = IPv4Endpoint.from_string("0.0.0.1:12000")
        instance = IPEndpoint.from_IPv4Endpoint(value)

        self.assertEqual(value, instance)
        self.assertEqual(value, instance.as_IPv4Endpoint())
        self.assertIsNone(instance.as_IPv6Endpoint())

        self.assertTrue(value == instance)
        self.assertFalse(value != instance)
        self.assertFalse(value < instance)
        self.assertTrue(value <= instance)
        self.assertFalse(value > instance)
        self.assertTrue(value >= instance)

    def test_IPEndpoint_from_IPv6Endpoint(self):
        value = IPv6Endpoint.from_string("[fe80::a:1]:12000")
        instance = IPEndpoint.from_IPv6Endpoint(value)

        self.assertEqual(value, instance)
        self.assertIsNone(instance.as_IPv4Endpoint())
        self.assertEqual(value, instance.as_IPv6Endpoint())

        self.assertTrue(value == instance)
        self.assertFalse(value != instance)
        self.assertFalse(value < instance)
        self.assertTrue(value <= instance)
        self.assertFalse(value > instance)
        self.assertTrue(value >= instance)

    def test_HostEndpoint(self):
        a = HostEndpoint.from_string("0.0.0.1:12000")
        b = HostEndpoint.from_string("[fe80::a:10]:12000")

        self.assertIsNot(a, b)

        self.assertFalse(a == b)
        self.assertTrue(a != b)
        self.assertTrue(a < b)
        self.assertTrue(a <= b)
        self.assertFalse(a > b)
        self.assertFalse(a >= b)

        self.assertNotEqual(hash(a), hash(b))
        self.assertEqual(1, len({a, a}))
        self.assertEqual(2, len({a, b}))

    def test_HostEndpoint_from_IPv4Endpoint(self):
        value = IPv4Endpoint.from_string("0.0.0.1:12000")
        instance = HostEndpoint.from_IPv4Endpoint(value)

        self.assertEqual(value, instance)
        self.assertEqual(value, instance.as_IPv4Endpoint())
        self.assertIsNone(instance.as_IPv6Endpoint())
        self.assertIsNone(instance.as_HostnameEndpoint())

        self.assertTrue(value == instance)
        self.assertFalse(value != instance)
        self.assertFalse(value < instance)
        self.assertTrue(value <= instance)
        self.assertFalse(value > instance)
        self.assertTrue(value >= instance)

    def test_HostEndpoint_from_IPv6Endpoint(self):
        value = IPv6Endpoint.from_string("[fe80::a:1]:12000")
        instance = HostEndpoint.from_IPv6Endpoint(value)

        self.assertEqual(value, instance)
        self.assertIsNone(instance.as_IPv4Endpoint())
        self.assertEqual(value, instance.as_IPv6Endpoint())
        self.assertIsNone(instance.as_HostnameEndpoint())

        self.assertTrue(value == instance)
        self.assertFalse(value != instance)
        self.assertFalse(value < instance)
        self.assertTrue(value <= instance)
        self.assertFalse(value > instance)
        self.assertTrue(value >= instance)

    def test_HostEndpoint_from_HostnameEndpoint(self):
        value = HostnameEndpoint.from_string("foo.bar.net:12000")
        instance = HostEndpoint.from_HostnameEndpoint(value)

        self.assertEqual(value, instance)
        self.assertIsNone(instance.as_IPv4Endpoint())
        self.assertIsNone(instance.as_IPv6Endpoint())
        self.assertEqual(value, instance.as_HostnameEndpoint())

        self.assertTrue(value == instance)
        self.assertFalse(value != instance)
        self.assertFalse(value < instance)
        self.assertTrue(value <= instance)
        self.assertFalse(value > instance)
        self.assertTrue(value >= instance)
