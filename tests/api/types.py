"""
Types API tests.
"""

from unittest import TestCase

from pyfreelan.api import native, ffi


class APITypesTests(TestCase):
    def test_IPv4Address_from_string_simple(self):
        result = native.freelan_IPv4Address_from_string("1.2.4.8")

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
