"""
Types API tests.
"""

from .. import MemoryTests

from pyfreelan.api import native, ffi


class APITypesTests(MemoryTests):
    def test_IPv4Address_from_string_simple(self):
        result = self.smartptr(
            native.freelan_IPv4Address_from_string("1.2.4.8"),
            native.freelan_IPv4Address_free,
        )
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
        value = self.smartptr(
            native.freelan_IPv4Address_from_string(str_value),
            native.freelan_IPv4Address_free,
        )
        result = self.smartptr(
            native.freelan_IPv4Address_to_string(value),
            native.freelan_free,
        )
        self.assertEqual(str_value, ffi.string(result))
