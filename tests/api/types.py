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
from pyfreelan.api.error import (
    FreeLANException,
    check_error_context,
)


class NativeTypeTests(TestCase):
    mocks = (
        'types_native',
        'types_ffi',
        'error_native',
        'error_ffi',
    )

    def setUp(self):
        super(NativeTypeTests, self).setUp()

        for patch_name in self.mocks:
            patcher = patch('pyfreelan.api.' + patch_name.replace('_', '.'))
            patcher_name = patch_name + '_patch'
            setattr(self, patcher_name, patcher)
            setattr(self, patch_name, patcher.start())

    def tearDown(self):
        for patch_name in reversed(self.mocks):
            patcher_name = patch_name + '_patch'
            patcher = getattr(self, patcher_name)
            patcher.stop()

    def test_instantiation(self):
        ectx = self.error_native.freelan_acquire_error_context()

        def freelan_error_context_get_error_category(_ectx):
            self.assertEqual(ectx, _ectx)
            return self.error_ffi.NULL

        self.error_native.freelan_error_context_get_error_category = freelan_error_context_get_error_category
        self.types_native.freelan_Foo_from_string.return_value = 42

        cls = NativeType('Foo')
        value = cls("myvalue")

        self.types_native.freelan_Foo_from_string.called_once_with(ectx, "myvalue")
        self.assertEqual(42, value._opaque_ptr)

    def test_check_error_context_no_error(self):
        ectx = self.error_native.freelan_acquire_error_context()

        def freelan_error_context_get_error_category(_ectx):
            self.assertEqual(ectx, _ectx)
            return self.error_ffi.NULL

        self.error_native.freelan_error_context_get_error_category = freelan_error_context_get_error_category

        @check_error_context
        def func(ectx):
            self.types_native.my_func(ectx)

        func()

        self.types_native.my_func.called_once_with(ectx)

    def test_check_error_context_error(self):
        ectx = self.error_native.freelan_acquire_error_context()

        def freelan_error_context_get_error_category(_ectx):
            self.assertEqual(ectx, _ectx)
            return "mycategory_ptr"

        def freelan_error_context_get_error_code(_ectx):
            self.assertEqual(ectx, _ectx)
            return 42

        def freelan_error_context_get_error_description(_ectx):
            self.assertEqual(ectx, _ectx)
            return "mydescription_ptr"

        def freelan_error_context_get_error_file(_ectx):
            self.assertEqual(ectx, _ectx)
            return "myfile_ptr"

        def freelan_error_context_get_error_line(_ectx):
            self.assertEqual(ectx, _ectx)
            return 123

        def ffi_string(ptr):
            self.assertIn(ptr, [
                "mycategory_ptr",
                "mydescription_ptr",
                "myfile_ptr",
            ])

            return ptr.split('_')[0]

        self.error_native.freelan_error_context_get_error_category = freelan_error_context_get_error_category
        self.error_native.freelan_error_context_get_error_code = freelan_error_context_get_error_code
        self.error_native.freelan_error_context_get_error_description = freelan_error_context_get_error_description
        self.error_native.freelan_error_context_get_error_file = freelan_error_context_get_error_file
        self.error_native.freelan_error_context_get_error_line = freelan_error_context_get_error_line
        self.error_ffi.string = ffi_string

        @check_error_context
        def func(ectx):
            self.types_native.my_func(ectx)

        with self.assertRaises(FreeLANException) as ctx:
            func()

        error_context = ctx.exception.error_context

        print self.error_native.mock_calls
        print self.error_ffi.mock_calls
        self.assertEqual("mycategory", error_context.category)
        self.assertEqual(42, error_context.code)
        self.assertEqual("mydescription", error_context.description)
        self.assertEqual("myfile", error_context.file)
        self.assertEqual(123, error_context.line)


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
    def test_IPv4Address_wrapper_instanciation(self):
        value = IPv4Address("9.0.0.1")
        self.assertEqual("9.0.0.1", str(value))

    def test_IPv4Address_wrapper_instanciation_failure(self):
        with self.assertRaises(FreeLANException) as ctx:
            IPv4Address("incorrect value")

        error_context = ctx.exception.error_context

        self.assertEqual("system", error_context.category)
        self.assertEqual(22, error_context.code)
        self.assertEqual("Invalid argument", error_context.description)
