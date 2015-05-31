"""
Types API tests.
"""

from unittest import TestCase
from functools import wraps
from contextlib import contextmanager

from mock import (
    patch,
    MagicMock,
)

from pyfreelan.api.types import (
    swallow_native_string,
    NativeType,
    IPv4Address,
    IPv6Address,
    Hostname,
    PortNumber,
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

    @disable_error_context_checks
    def test_wrapper_init_calls_from_string(self, ectx):
        wrapper = NativeType.create_wrapper('foo')
        instance = MagicMock(spec=wrapper)
        from_string = self.native.freelan_foo_from_string

        wrapper.__init__(instance, "mystr")

        from_string.assert_called_once_with(ectx, "mystr")
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
        self.assertEqual(equal(native_ptr_a, native_ptr_b), result)

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
        self.assertEqual(less_than(native_ptr_a, native_ptr_b), result)

    def test_wrapper_repr(self):
        wrapper = NativeType.create_wrapper('foo')
        instance = MagicMock(spec=wrapper)
        instance.__class__.__name__ = 'MyClass'
        native_ptr = MagicMock()
        native_ptr.__str__.return_value = '0x12345678'
        instance._opaque_ptr = native_ptr

        result = wrapper.__repr__(instance)

        self.assertEqual('MyClass(0x12345678)', result)

    def test_wrapper_comparison(self):
        def from_string(_, s):
            return s

        def equal(lhs, rhs):
            self.assertEqual({"a", "b"}, {lhs, rhs})
            return lhs == rhs

        def less_than(lhs, rhs):
            self.assertEqual({"a", "b"}, {lhs, rhs})
            return lhs < rhs

        self.native.freelan_foo_from_string = from_string
        self.native.freelan_foo_equal = equal
        self.native.freelan_foo_less_than = less_than

        wrapper = NativeType.create_wrapper('foo')
        instance_a = wrapper("a")
        instance_b = wrapper("b")

        self.assertFalse(instance_a == instance_b)
        self.assertTrue(instance_a != instance_b)
        self.assertFalse(instance_a > instance_b)
        self.assertFalse(instance_a >= instance_b)
        self.assertTrue(instance_a < instance_b)
        self.assertTrue(instance_a <= instance_b)

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
    def test_IPv4Address(self):
        a = IPv4Address("0.0.0.1")
        b = IPv4Address("0.0.0.2")

        self.assertIsNot(a, b)
        self.assertNotEqual(hash(a), hash(b))
        self.assertNotEqual(a, b)
        self.assertLess(a, b)
        self.assertEqual(1, len({a, a}))
        self.assertEqual(2, len({a, b}))

    def test_IPv6Address(self):
        a = IPv6Address("ffe0::abcd")
        b = IPv6Address("ffe0::abce")

        self.assertIsNot(a, b)
        self.assertNotEqual(hash(a), hash(b))
        self.assertNotEqual(a, b)
        self.assertLess(a, b)
        self.assertEqual(1, len({a, a}))
        self.assertEqual(2, len({a, b}))

    def test_Hostname(self):
        a = Hostname("my.host.name1")
        b = Hostname("my.host.name2")

        self.assertIsNot(a, b)
        self.assertNotEqual(hash(a), hash(b))
        self.assertNotEqual(a, b)
        self.assertLess(a, b)
        self.assertEqual(1, len({a, a}))
        self.assertEqual(2, len({a, b}))

    def test_PortNumber(self):
        a = PortNumber("12000")
        b = PortNumber("12001")

        self.assertIsNot(a, b)
        self.assertNotEqual(hash(a), hash(b))
        self.assertNotEqual(a, b)
        self.assertLess(a, b)
        self.assertEqual(1, len({a, a}))
        self.assertEqual(2, len({a, b}))

    def test_PortNumber_from_integer(self):
        a = PortNumber("12000")
        b = PortNumber(12000)

        self.assertEqual(a, b)

    def test_PortNumber_to_integer(self):
        instance = PortNumber("12000")

        self.assertEqual(12000, int(instance))
