"""
Types API tests.
"""

from unittest import TestCase

from mock import (
    patch,
    MagicMock,
)

from pyfreelan.api import ffi
from pyfreelan.api.types import (
    swallow_native_string,
    NativeType,
)
from pyfreelan.api.error import ErrorContext


class SwallowNativeStringTests(TestCase):
    def test_wrapped_function_is_accessible(self):
        def myfunc():
            pass

        decorated = swallow_native_string(myfunc)
        self.assertEqual(decorated.wrapped, myfunc)

    def test_non_null_value(self):
        value = MagicMock()

        @swallow_native_string
        def myfunc(*args, **kwargs):
            self.assertEqual((1,), args)
            self.assertEqual({'b': 2}, kwargs)
            return value

        with patch('pyfreelan.api.error.ffi.string') as string_mock:
            with patch(
                'pyfreelan.api.error.native.freelan_free',
            ) as freelan_free_mock:
                result = myfunc(1, b=2)

        string_mock.assert_called_once_with(value)
        freelan_free_mock.assert_called_once_with(value)
        self.assertEqual(string_mock(value), result)

    def test_null_value(self):
        @swallow_native_string
        def myfunc():
            return ffi.NULL

        result = myfunc()

        self.assertEqual(None, result)


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

    def test_wrapper_init_calls_from_string(self):
        wrapper = NativeType.create_wrapper('foo')
        instance = MagicMock(spec=wrapper)
        ectx = MagicMock(spec=ErrorContext)
        from_string = self.native.freelan_foo_from_string

        wrapper.__init__.wrapped(instance, "mystr", ectx)

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

    def test_wrapper_str_calls_to_string(self):
        wrapper = NativeType.create_wrapper('foo')
        instance = MagicMock(spec=wrapper)
        native_ptr = MagicMock()
        instance._opaque_ptr = native_ptr
        ectx = MagicMock(spec=ErrorContext)
        to_string = self.native.freelan_foo_to_string

        result = wrapper.__str__.wrapped.wrapped(instance, ectx)

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
