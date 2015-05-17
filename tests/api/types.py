"""
Types API tests.
"""

from unittest import TestCase
from .. import NativeTests

from mock import patch

from pyfreelan.api import (
    native,
    ffi,
)
from pyfreelan.api.types import (
    NativeType,
    IPv4Address,
)


class NativeTypeTests(TestCase):
    def setUp(self):
        super(NativeTypeTests, self).setUp()

        with patch('pyfreelan.api.types.native') as native:
            self.cls = NativeType('Foo')
            self.native = native

    def test_instantiation(self):
        self.native.freelan_Foo_from_string.return_value = 42

        value = self.cls("myvalue")

        self.native.freelan_Foo_from_string.called_once_with("myvalue")
        self.assertEqual(42, value._opaque_ptr)


class APINativeTypesTests(NativeTests):
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


class APITypesTests(TestCase):
    def test_IPv4Address_wrapper(self):
        value = IPv4Address("9.0.0.1")
        self.assertEqual("9.0.0.1", str(value))
