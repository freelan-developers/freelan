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
    ErrorContext,
)


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

        instance1 = ErrorContext.get_current.__func__(MyClass)
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

        self.assertTrue(
            hasattr(MyClass.thread_local_data, 'freelan_error_context'),
        )

        ErrorContext.clear_current.__func__(MyClass)

        self.assertFalse(
            hasattr(MyClass.thread_local_data, 'freelan_error_context'),
        )
