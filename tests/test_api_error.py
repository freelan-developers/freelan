"""
Test the API error functions and helpers.
"""

from __future__ import unicode_literals

from unittest import TestCase
from threading import Thread
from six.moves.queue import Queue
from mock import (
    patch,
    MagicMock,
)

from pyfreelan.api.error import (
    ffi,
    from_native_string,
    convert_native_string,
    ErrorContext,
    FreeLANException,
    inject_error_context,
    check_error_context,
)


class FromNativeStringTests(TestCase):

    def test_non_null_value_without_free_on_success(self):
        value = ffi.new('const char[]', "foo".encode('utf-8'))
        result = from_native_string(value, free_on_success=False)

        self.assertEqual("foo", result)

    @patch('pyfreelan.api.error.native')
    def test_non_null_value_with_free_on_success(self, native_mock):
        value = ffi.new('const char[]', "foo".encode('utf-8'))
        result = from_native_string(value, free_on_success=True)

        self.assertEqual("foo", result)
        native_mock.freelan_free.assert_called_once_with(value)

    def test_null_value(self):
        value = ffi.NULL
        result = from_native_string(value)

        self.assertEqual(None, result)


class ConvertNativeStringTests(TestCase):

    def test_from_native_string_is_called(self):
        value = MagicMock()

        @convert_native_string
        def func(*args, **kwargs):
            return value

        with patch(
            'pyfreelan.api.error.from_native_string',
        ) as from_native_string_mock:
            func()

        from_native_string_mock.assert_called_once_with(
            value,
            free_on_success=False,
        )

    def test_all_arguments_pass_through(self):
        value = MagicMock()

        @convert_native_string
        def func(*args, **kwargs):
            self.assertEqual((42,), args)
            self.assertEqual({'foo': 'bar'}, kwargs)
            return value

        with patch(
            'pyfreelan.api.error.from_native_string',
        ) as from_native_string_mock:
            result = func(42, foo='bar')

        self.assertEqual(from_native_string_mock(value), result)

    def test_wrapped_function_is_accessible(self):
        def myfunc():
            pass

        decorated = convert_native_string(myfunc)
        self.assertEqual(decorated.wrapped, myfunc)


class ErrorContextTests(TestCase):

    @patch('pyfreelan.api.error.native')
    def test_get_current_returns_a_new_instance_on_first_call(
        self,
        native_mock,
    ):
        instance = ErrorContext.get_current()

        self.assertNotEqual(None, instance)

        # We must clear the instance from the thread local data or it will leak
        # to other tests.
        ErrorContext.clear_current()

    @patch('pyfreelan.api.error.native')
    def test_get_current_returns_the_same_instance_on_second_call(
        self,
        native_mock,
    ):
        instance = ErrorContext.get_current()
        instance_2 = ErrorContext.get_current()

        self.assertIs(instance, instance_2)

        # We must clear the instance from the thread local data or it will leak
        # to other tests.
        ErrorContext.clear_current()

    @patch('pyfreelan.api.error.native')
    def test_get_current_returns_different_instances_for_different_threads(
        self,
        native_mock,
    ):
        queue = Queue()

        def target():
            instance = ErrorContext.get_current()
            queue.put(instance)

        thread1 = Thread(target=target)
        thread2 = Thread(target=target)
        thread1.start()
        thread2.start()
        thread1.join()
        thread2.join()

        instance_1, instance_2 = queue.get(), queue.get()
        self.assertNotEqual(instance_1, instance_2)

    @patch('pyfreelan.api.error.native')
    def test_clear_current_clears_the_cached_value(self, native_mock):
        instance = ErrorContext.get_current()
        ErrorContext.clear_current()
        instance_2 = ErrorContext.get_current()

        self.assertNotEqual(instance, instance_2)

        ErrorContext.clear_current()

    @patch('pyfreelan.api.error.native')
    def test_init_creates_an_native_error_context(self, native_mock):
        instance = ErrorContext()

        native_mock.freelan_acquire_error_context.assert_called_once_with()
        self.assertEqual(
            native_mock.freelan_acquire_error_context(),
            instance._opaque_ptr,
        )

    @patch('pyfreelan.api.error.native')
    def test_del_releases_the_native_error_context(self, native_mock):
        native_ptr = MagicMock()
        instance = ErrorContext()
        instance._opaque_ptr = native_ptr
        del instance

        native_mock.freelan_release_error_context.assert_called_once_with(
            native_ptr,
        )

    @patch('pyfreelan.api.error.native')
    def test_reset_resets_the_native_error_context(self, native_mock):
        instance = ErrorContext()
        instance.reset()

        native_mock.freelan_error_context_reset.assert_called_once_with(
            instance._opaque_ptr,
        )

    @patch('pyfreelan.api.error.native')
    def test_category_property(self, native_mock):
        native_mock.freelan_error_context_get_error_category.return_value = \
            ffi.new("const char[]", "foo".encode('utf-8'))
        instance = ErrorContext()

        self.assertEqual("foo", instance.category)

        native_mock.freelan_error_context_get_error_category.\
            assert_called_once_with(instance._opaque_ptr)

    @patch('pyfreelan.api.error.native')
    def test_code_property(self, native_mock):
        native_mock.freelan_error_context_get_error_code.return_value = 42
        instance = ErrorContext()

        self.assertEqual(42, instance.code)

        native_mock.freelan_error_context_get_error_code.\
            assert_called_once_with(instance._opaque_ptr)

    @patch('pyfreelan.api.error.native')
    def test_description_property(self, native_mock):
        native_mock.freelan_error_context_get_error_description.return_value = \
            ffi.new("const char[]", "foo".encode('utf-8'))
        instance = ErrorContext()

        self.assertEqual("foo", instance.description)

        native_mock.freelan_error_context_get_error_description.\
            assert_called_once_with(instance._opaque_ptr)

    @patch('pyfreelan.api.error.native')
    def test_file_property(self, native_mock):
        native_mock.freelan_error_context_get_error_file.return_value = \
            ffi.new("const char[]", "foo".encode('utf-8'))
        instance = ErrorContext()

        self.assertEqual("foo", instance.file)

        native_mock.freelan_error_context_get_error_file.\
            assert_called_once_with(instance._opaque_ptr)

    @patch('pyfreelan.api.error.native')
    def test_line_property(self, native_mock):
        native_mock.freelan_error_context_get_error_line.return_value = 42
        instance = ErrorContext()

        self.assertEqual(42, instance.line)

        native_mock.freelan_error_context_get_error_line.\
            assert_called_once_with(instance._opaque_ptr)

    @patch('pyfreelan.api.error.native')
    def test_non_zeroness_without_category(self, native_mock):
        native_mock.freelan_error_context_get_error_category.return_value = \
            ffi.NULL
        instance = ErrorContext()

        self.assertFalse(instance)

    @patch('pyfreelan.api.error.native')
    @patch('pyfreelan.api.error.ffi')
    def test_non_zeroness_with_category(self, native_mock, ffi_mock):
        instance = ErrorContext()

        self.assertTrue(instance)

    @patch('pyfreelan.api.error.native')
    @patch('pyfreelan.api.error.ffi')
    def test_as_context_manager(self, native_mock, ffi_mock):
        instance = ErrorContext()

        with self.assertRaises(FreeLANException):
            with instance as ptr:
                self.assertEqual(instance._opaque_ptr, ptr)
                native_mock.freelan_error_context_reset(ptr)

    @patch('pyfreelan.api.error.native')
    def test_raise_for_error_does_not_raise_without_error(self, native_mock):
        native_mock.freelan_error_context_get_error_category.return_value = \
            ffi.NULL
        instance = ErrorContext()
        instance.raise_for_error()

    @patch('pyfreelan.api.error.native')
    @patch('pyfreelan.api.error.ffi')
    def test_raise_for_error_raises_on_error(self, native_mock, ffi_mock):
        instance = ErrorContext()

        with self.assertRaises(FreeLANException) as ctx:
            instance.raise_for_error()

        self.assertEqual(instance, ctx.exception.error_context)


class FreeLANExceptionTests(TestCase):

    def test_init_stores_the_error_context(self):
        error_context = MagicMock()
        instance = FreeLANException(error_context=error_context)

        self.assertEqual(error_context, instance.error_context)

    def test_str_without_file_information(self):
        instance = FreeLANException(error_context=MagicMock())
        instance.error_context.file = None
        instance.error_context.line = None
        instance.error_context.category = "mycategory"
        instance.error_context.code = 42
        instance.error_context.description = "mydescription"

        result = str(instance)

        self.assertEqual(
            "mycategory:42 - mydescription",
            result,
        )

    def test_str_with_file_information(self):
        instance = FreeLANException(error_context=MagicMock())
        instance.error_context.file = "myfile"
        instance.error_context.line = 123
        instance.error_context.category = "mycategory"
        instance.error_context.code = 42
        instance.error_context.description = "mydescription"

        result = str(instance)

        self.assertEqual(
            "mycategory:42 - mydescription (myfile:123)",
            result,
        )


class InjectErrorContextTests(TestCase):

    def test_keyword_injection(self):
        inital_kwargs = {
            'a': 'b',
            1: 2,
        }
        kwargs = inital_kwargs

        with patch(
            'pyfreelan.api.error.ErrorContext.get_current',
        ) as get_current_mock:
            with inject_error_context('foo', kwargs) as new_kwargs:
                self.assertEqual(
                    {'a': 'b', 1: 2, 'foo': get_current_mock().__enter__()},
                    new_kwargs,
                )

        self.assertEqual(inital_kwargs, kwargs)


class CheckErrorContextTests(TestCase):

    def test_inject_error_context_is_called(self):
        value = MagicMock()

        @check_error_context
        def func(*args, **kwargs):
            return value

        with patch(
            'pyfreelan.api.error.inject_error_context',
        ) as inject_error_context_mock:
            func(1, a='b')

        inject_error_context_mock.assert_called_once_with('ectx', {'a': 'b'})

    def test_all_arguments_pass_through(self):
        value = MagicMock()

        @check_error_context
        def func(*args, **kwargs):
            self.assertEqual((42,), args)
            self.assertEqual({'a': 'b'}, kwargs)
            return value

        with patch(
            'pyfreelan.api.error.inject_error_context',
        ) as inject_error_context_mock:
            inject_error_context_mock().__enter__.return_value = {'a': 'b'}
            result = func(42, foo='bar')

        self.assertEqual(value, result)

    def test_wrapped_function_is_accessible(self):
        def myfunc():
            pass

        decorated = check_error_context(myfunc)
        self.assertEqual(decorated.wrapped, myfunc)
