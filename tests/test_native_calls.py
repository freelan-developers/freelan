"""
Test direct calling of the C methods.
"""

from unittest import TestCase
from pyfreelan.api import (
    native,
    ffi,
)


class NativeCallsTests(TestCase):
    def setUp(self):
        self.ectx = native.freelan_acquire_error_context()

    def tearDown(self):
        native.freelan_release_error_context(self.ectx)
        self.ectx = None

    def test_malloc(self):
        result = native.freelan_malloc(6)

        self.assertNotEqual(ffi.NULL, result)

        native.freelan_free(result)

    def test_realloc(self):
        buf = native.freelan_malloc(6)
        result = native.freelan_realloc(buf, 12)

        self.assertNotEqual(ffi.NULL, result)

        native.freelan_free(result)

    def test_strdup(self):
        result = native.freelan_strdup("freelan")

        self.assertEqual("freelan", ffi.string(result))

        native.freelan_free(result)

    def test_IPv4Address_from_string_simple(self):
        result = native.freelan_IPv4Address_from_string(self.ectx, "1.2.4.8")

        self.assertNotEqual(ffi.NULL, result)

        native.freelan_IPv4Address_free(result)

    def test_IPv4Address_from_string_truncated(self):
        result = native.freelan_IPv4Address_from_string(self.ectx, "127.1")

        self.assertEqual(ffi.NULL, result)

    def test_IPv4Address_from_string_incorrect_value(self):
        result = native.freelan_IPv4Address_from_string(self.ectx, "incorrect value")

        self.assertEqual(ffi.NULL, result)

    def test_IPv4Address_from_string_empty_value(self):
        result = native.freelan_IPv4Address_from_string(self.ectx, "")

        self.assertEqual(ffi.NULL, result)

    def test_IPv4Address_to_string_simple(self):
        str_value = "1.2.4.8"
        value = native.freelan_IPv4Address_from_string(self.ectx, str_value)
        result = native.freelan_IPv4Address_to_string(self.ectx, value)
        native.freelan_IPv4Address_free(value)

        self.assertEqual(str_value, ffi.string(result))

        native.freelan_free(result)
