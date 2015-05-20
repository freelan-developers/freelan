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
        self.addCleanup(native.freelan_free, result)

        self.assertNotEqual(ffi.NULL, result)

    def test_realloc(self):
        buf = native.freelan_malloc(6)
        result = native.freelan_realloc(buf, 12)
        self.addCleanup(native.freelan_free, result)

        self.assertNotEqual(ffi.NULL, result)

    def test_strdup(self):
        result = native.freelan_strdup("freelan")
        self.addCleanup(native.freelan_free, result)

        self.assertEqual("freelan", ffi.string(result))

    def test_acquire_error_context(self):
        result = native.freelan_acquire_error_context()
        self.addCleanup(native.freelan_release_error_context, result)

        self.assertNotEqual(ffi.NULL, result)

    def test_error_context_reset(self):
        ectx = native.freelan_acquire_error_context()
        self.addCleanup(native.freelan_release_error_context, ectx)

        native.freelan_error_context_reset(ectx)

    def test_error_context_get_error_category(self):
        ectx = native.freelan_acquire_error_context()
        self.addCleanup(native.freelan_release_error_context, ectx)

        result = native.freelan_error_context_get_error_category(ectx)
        self.assertEqual(ffi.NULL, result)

    def test_error_context_get_error_code(self):
        ectx = native.freelan_acquire_error_context()
        self.addCleanup(native.freelan_release_error_context, ectx)

        result = native.freelan_error_context_get_error_code(ectx)
        self.assertEqual(0, result)

    def test_error_context_get_error_description(self):
        ectx = native.freelan_acquire_error_context()
        self.addCleanup(native.freelan_release_error_context, ectx)

        result = native.freelan_error_context_get_error_description(ectx)
        self.assertEqual(ffi.NULL, result)

    def test_error_context_get_error_file(self):
        ectx = native.freelan_acquire_error_context()
        self.addCleanup(native.freelan_release_error_context, ectx)

        result = native.freelan_error_context_get_error_file(ectx)
        self.assertEqual(ffi.NULL, result)

    def test_error_context_get_error_line(self):
        ectx = native.freelan_acquire_error_context()
        self.addCleanup(native.freelan_release_error_context, ectx)

        result = native.freelan_error_context_get_error_line(ectx)
        self.assertEqual(0, result)

    def test_IPv4Address_from_string_simple(self):
        result = native.freelan_IPv4Address_from_string(self.ectx, "1.2.4.8")
        self.addCleanup(native.freelan_IPv4Address_free, result)

        self.assertNotEqual(ffi.NULL, result)

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
        self.addCleanup(native.freelan_IPv4Address_free, value)

        result = native.freelan_IPv4Address_to_string(self.ectx, value)
        self.addCleanup(native.freelan_free, result)

        self.assertEqual(str_value, ffi.string(result))
