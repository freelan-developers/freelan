"""
Types API tests.
"""

from unittest import TestCase

from pyfreelan.api import native, ffi
from contextlib import contextmanager


@contextmanager
def free(value, method):
    try:
        yield
    finally:
        method(value)


class APITypesTests(TestCase):
    def test_IPv4Address_from_string_simple(self):
        result = native.freelan_IPv4Address_from_string("1.2.4.8")

        with free(result, native.freelan_IPv4Address_free):
            self.assertNotEqual(ffi.NULL, result)

    def test_IPv4Address_from_string_truncated(self):
        result = native.freelan_IPv4Address_from_string("127.1")

        self.assertEqual(ffi.NULL, result)

    def test_IPv4Address_from_string_incorrect_value(self):
        result = native.freelan_IPv4Address_from_string("incorrect value")

        self.assertEqual(ffi.NULL, result)

    def test_IPv4Address_from_string_empty_value(self):
        result = native.freelan_IPv4Address_from_string("")

        self.assertEqual(ffi.NULL, result)

    def test_IPv4Address_to_string_simple(self):
        str_value = "1.2.4.8"
        value = native.freelan_IPv4Address_from_string(str_value)

        with free(value, native.freelan_IPv4Address_free):
            result = native.freelan_IPv4Address_to_string(value)

            with free(result, native.freelan_free):
                self.assertEqual(str_value, ffi.string(result))
