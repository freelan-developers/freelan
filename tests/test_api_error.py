"""
Test the API error functions and helpers.
"""

from unittest import TestCase
from threading import (
    Thread,
    local,
)
from Queue import Queue
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
        value = MagicMock()

        with patch('pyfreelan.api.error.ffi.string') as string_mock:
            with patch(
                'pyfreelan.api.error.native.freelan_free',
            ) as freelan_free_mock:
                result = from_native_string(value, free_on_success=False)

        string_mock.assert_called_once_with(value)
        self.assertEqual([], freelan_free_mock.mock_calls)
        self.assertEqual(string_mock(value), result)

    def test_non_null_value_with_free_on_success(self):
        value = MagicMock()

        with patch('pyfreelan.api.error.ffi.string') as string_mock:
            with patch(
                'pyfreelan.api.error.native.freelan_free',
            ) as freelan_free_mock:
                result = from_native_string(value, free_on_success=True)

        string_mock.assert_called_once_with(value)
        freelan_free_mock.assert_called_once_with(value)
        self.assertEqual(string_mock(value), result)

    def test_null_value(self):
        value = ffi.NULL

        with patch('pyfreelan.api.error.ffi.string') as string_mock:
            result = from_native_string(value)

        self.assertEqual([], string_mock.mock_calls)
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
    def test_get_current_returns_a_new_instance_on_first_call(self):
        class MyClass(object):
            thread_local_data = local()

        self.assertFalse(
            hasattr(MyClass.thread_local_data, 'freelan_error_context'),
        )

        instance = ErrorContext.get_current.__func__(MyClass)

        self.assertNotEqual(None, instance)
        self.assertEqual(
            MyClass.thread_local_data.freelan_error_context,
            instance,
        )

    def test_get_current_returns_the_same_instance_on_second_call(self):
        class MyClass(object):
            thread_local_data = local()

        instance1 = MyClass()
        MyClass.thread_local_data.freelan_error_context = instance1

        instance2 = ErrorContext.get_current.__func__(MyClass)

        self.assertEqual(instance1, instance2)

    def test_get_current_returns_different_instances_for_different_threads(self):
        class MyClass(object):
            thread_local_data = local()

        queue = Queue()

        def target():
            instance = ErrorContext.get_current.__func__(MyClass)
            queue.put(instance)

        thread1 = Thread(target=target)
        thread2 = Thread(target=target)
        thread1.start()
        thread2.start()
        thread1.join()
        thread2.join()

        instance1, instance2 = queue.get(), queue.get()
        self.assertNotEqual(instance1, instance2)

    def test_clear_current_clears_the_cached_value(self):
        class MyClass(object):
            thread_local_data = local()

        instance1 = MyClass()
        MyClass.thread_local_data.freelan_error_context = instance1

        ErrorContext.clear_current.__func__(MyClass)

        self.assertFalse(
            hasattr(MyClass.thread_local_data, 'freelan_error_context'),
        )

    def test_init_creates_an_native_error_context(self):
        instance = MagicMock()

        with patch(
            'pyfreelan.api.error.native.freelan_acquire_error_context',
        ) as freelan_acquire_error_context_mock:
            ErrorContext.__init__.__func__(instance)

        freelan_acquire_error_context_mock.assert_called_once_with()
        self.assertEqual(
            freelan_acquire_error_context_mock(),
            instance._opaque_ptr,
        )

    def test_del_releases_the_native_error_context(self):
        instance = MagicMock()
        native_ptr = MagicMock()
        instance._opaque_ptr = native_ptr

        with patch(
            'pyfreelan.api.error.native.freelan_release_error_context',
        ) as freelan_release_error_context_mock:
            ErrorContext.__del__.__func__(instance)

        freelan_release_error_context_mock.assert_called_once_with(
            native_ptr,
        )
        self.assertEqual(
            None,
            instance._opaque_ptr,
        )

    def test_reset_resets_the_native_error_context(self):
        instance = MagicMock()
        native_ptr = MagicMock()
        instance._opaque_ptr = native_ptr

        with patch(
            'pyfreelan.api.error.native.freelan_error_context_reset',
        ) as freelan_error_context_reset_mock:
            ErrorContext.reset.__func__(instance)

        freelan_error_context_reset_mock.assert_called_once_with(
            native_ptr,
        )

    @patch('pyfreelan.api.error.from_native_string')
    def test_category_property(self, from_native_string_mock):
        instance = MagicMock()
        native_ptr = MagicMock()
        instance._opaque_ptr = native_ptr

        with patch(
            'pyfreelan.api.error.native.'
            'freelan_error_context_get_error_category',
        ) as freelan_error_context_get_error_category_mock:
            result = ErrorContext.category.fget(instance)

        freelan_error_context_get_error_category_mock.assert_called_once_with(
            native_ptr,
        )
        self.assertEqual(
            from_native_string_mock(
                freelan_error_context_get_error_category_mock(native_ptr),
            ),
            result,
        )

    def test_code_property(self):
        instance = MagicMock()
        native_ptr = MagicMock()
        instance._opaque_ptr = native_ptr

        with patch(
            'pyfreelan.api.error.native.'
            'freelan_error_context_get_error_code',
        ) as freelan_error_context_get_error_code_mock:
            result = ErrorContext.code.fget(instance)

        freelan_error_context_get_error_code_mock.assert_called_once_with(
            native_ptr,
        )
        self.assertEqual(
            freelan_error_context_get_error_code_mock(native_ptr),
            result,
        )

    @patch('pyfreelan.api.error.from_native_string')
    def test_description_property(self, from_native_string_mock):
        instance = MagicMock()
        native_ptr = MagicMock()
        instance._opaque_ptr = native_ptr

        with patch(
            'pyfreelan.api.error.native.'
            'freelan_error_context_get_error_description',
        ) as freelan_error_context_get_error_description_mock:
            result = ErrorContext.description.fget(instance)

        freelan_error_context_get_error_description_mock.assert_called_once_with(
            native_ptr,
        )
        self.assertEqual(
            from_native_string_mock(
                freelan_error_context_get_error_description_mock(native_ptr),
            ),
            result,
        )

    @patch('pyfreelan.api.error.from_native_string')
    def test_file_property(self, from_native_string_mock):
        instance = MagicMock()
        native_ptr = MagicMock()
        instance._opaque_ptr = native_ptr

        with patch(
            'pyfreelan.api.error.native.'
            'freelan_error_context_get_error_file',
        ) as freelan_error_context_get_error_file_mock:
            result = ErrorContext.file.fget(instance)

        freelan_error_context_get_error_file_mock.assert_called_once_with(
            native_ptr,
        )
        self.assertEqual(
            from_native_string_mock(
                freelan_error_context_get_error_file_mock(native_ptr),
            ),
            result,
        )

    def test_line_property(self):
        instance = MagicMock()
        native_ptr = MagicMock()
        instance._opaque_ptr = native_ptr

        with patch(
            'pyfreelan.api.error.native.'
            'freelan_error_context_get_error_line',
        ) as freelan_error_context_get_error_line_mock:
            result = ErrorContext.line.fget(instance)

        freelan_error_context_get_error_line_mock.assert_called_once_with(
            native_ptr,
        )
        self.assertEqual(
            freelan_error_context_get_error_line_mock(native_ptr),
            result,
        )

    def test_non_zeroness_without_category(self):
        instance = MagicMock(spec=ErrorContext)
        instance.category = None

        self.assertFalse(ErrorContext.__nonzero__(instance))

    def test_non_zeroness_with_category(self):
        instance = MagicMock(spec=ErrorContext)
        instance.category = "somecategory"

        self.assertTrue(ErrorContext.__nonzero__(instance))

    def test_enter_resets_the_error_context(self):
        instance = MagicMock(spec=ErrorContext)
        instance._opaque_ptr = MagicMock()

        result = ErrorContext.__enter__(instance)

        instance.reset.assert_called_once_with()
        self.assertEqual(instance._opaque_ptr, result)

    def test_exit_checks_if_an_error_must_be_raised(self):
        instance = MagicMock(spec=ErrorContext)

        ErrorContext.__exit__(instance, None, None, None)

        instance.raise_for_error.assert_called_once_with()

    def test_raise_for_error_does_not_raise_without_error(self):
        instance = MagicMock(spec=ErrorContext)
        instance.__nonzero__.return_value = False

        ErrorContext.raise_for_error(instance)

    def test_raise_for_error_raises_on_error(self):
        instance = MagicMock(spec=ErrorContext)
        instance.__nonzero__.return_value = True

        class MyException(Exception):
            def __init__(self, error_context):
                self.error_context = error_context

        with patch(
            'pyfreelan.api.error.FreeLANException',
            MyException,
        ):
            with self.assertRaises(MyException) as ctx:
                ErrorContext.raise_for_error(instance)

        self.assertEqual(instance, ctx.exception.error_context)


class FreeLANExceptionTests(TestCase):
    def test_init_stores_the_error_context(self):
        error_context = MagicMock()
        instance = FreeLANException(error_context=error_context)

        self.assertEqual(error_context, instance.error_context)

    def test_str_without_file_information(self):
        instance = MagicMock(spec=FreeLANException)
        instance.error_context = MagicMock(spec=ErrorContext)
        instance.error_context.file = None
        instance.error_context.line = None
        instance.error_context.category = "mycategory"
        instance.error_context.code = 42
        instance.error_context.description = "mydescription"

        result = FreeLANException.__str__(instance)

        self.assertEqual(
            "mycategory:42 - mydescription",
            result,
        )

    def test_str_with_file_information(self):
        instance = MagicMock(spec=FreeLANException)
        instance.error_context = MagicMock(spec=ErrorContext)
        instance.error_context.file = "myfile"
        instance.error_context.line = 123
        instance.error_context.category = "mycategory"
        instance.error_context.code = 42
        instance.error_context.description = "mydescription"

        result = FreeLANException.__str__(instance)

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
