"""
Test direct calling of the C methods.
"""

from unittest import TestCase
from mock import MagicMock
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

    def test_set_logging_callback(self):
        @ffi.callback(
            "int (FreeLANLogLevel, FreeLANTimestamp, char *, char *, size_t, "
            "struct FreeLANLogPayload *, char *, unsigned int)",
        )
        def callback(
            level,
            timestamp,
            domain,
            code,
            payload_size,
            payload,
            file,
            line,
        ):
            return 1

        self.addCleanup(native.freelan_set_logging_callback, ffi.NULL)

        native.freelan_set_logging_callback(callback)

    def test_log_level(self):
        level = native.FREELAN_LOG_LEVEL_DEBUG
        native.freelan_set_log_level(level)
        result = native.freelan_get_log_level()

        self.assertEqual(level, result)

    def test_log_simple(self):
        self.addCleanup(native.freelan_set_logging_callback, ffi.NULL)

        callback = MagicMock(return_value=1)
        c_callback = ffi.callback(
            "int (FreeLANLogLevel, FreeLANTimestamp, char *, char *, size_t, "
            "struct FreeLANLogPayload *, char *, unsigned int)",
        )(callback)

        native.freelan_set_logging_callback(c_callback)

        domain = ffi.new("char[]", "mydomain")
        code = ffi.new("char[]", "mycode")
        file = ffi.new("char[]", "myfile")
        result = native.freelan_log(
            native.FREELAN_LOG_LEVEL_IMPORTANT,
            42,
            domain,
            code,
            0,
            ffi.NULL,
            file,
            123,
        )
        callback.assert_called_once_with(
            native.FREELAN_LOG_LEVEL_IMPORTANT,
            42,
            domain,
            code,
            0,
            ffi.NULL,
            file,
            123,
        )
        self.assertEqual(1, result)

    def test_log_extended(self):
        self.addCleanup(native.freelan_set_logging_callback, ffi.NULL)

        context = {'call_count': 0}
        p_domain = ffi.new("char[]", "mydomain")
        p_code = ffi.new("char[]", "mycode")
        p_file = ffi.new("char[]", "myfile")

        @ffi.callback(
            "int (FreeLANLogLevel, FreeLANTimestamp, char *, char *, size_t, "
            "struct FreeLANLogPayload *, char *, unsigned int)",
        )
        def callback(
            level,
            timestamp,
            domain,
            code,
            payload_size,
            payload,
            file,
            line,
        ):
            self.assertEqual(native.FREELAN_LOG_LEVEL_IMPORTANT, level)
            self.assertEqual(42, timestamp)
            self.assertEqual(p_domain, domain)
            self.assertEqual(p_code, code)
            self.assertEqual(4, payload_size)
            self.assertNotEqual(ffi.NULL, payload)
            self.assertEqual(p_file, file)
            self.assertEqual(123, line)

            self.assertEqual("a", ffi.string(payload[0].key))
            self.assertEqual(
                native.FREELAN_LOG_PAYLOAD_TYPE_STRING,
                payload[0].type,
            )
            self.assertEqual("hello", ffi.string(payload[0].value.as_string))

            self.assertEqual("b", ffi.string(payload[1].key))
            self.assertEqual(
                native.FREELAN_LOG_PAYLOAD_TYPE_INTEGER,
                payload[1].type,
            )
            self.assertEqual(42, payload[1].value.as_integer)

            self.assertEqual("c", ffi.string(payload[2].key))
            self.assertEqual(
                native.FREELAN_LOG_PAYLOAD_TYPE_FLOAT,
                payload[2].type,
            )
            self.assertAlmostEqual(3.14, payload[2].value.as_float)

            self.assertEqual("d", ffi.string(payload[3].key))
            self.assertEqual(
                native.FREELAN_LOG_PAYLOAD_TYPE_BOOLEAN,
                payload[3].type,
            )
            self.assertEqual(1, payload[3].value.as_boolean)

            context['call_count'] += 1

            return 1

        native.freelan_set_logging_callback(callback)

        log = native.freelan_log_start(
            native.FREELAN_LOG_LEVEL_IMPORTANT,
            42,
            p_domain,
            p_code,
            p_file,
            123,
        )
        native.freelan_log_attach_string(log, "a", "hello")
        native.freelan_log_attach_integer(log, "b", 42)
        native.freelan_log_attach_float(log, "c", 3.14)
        native.freelan_log_attach_boolean(log, "d", True)
        result = native.freelan_log_complete(log)

        self.assertEqual(1, context['call_count'])
        self.assertEqual(1, result)

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

    def test_IPv4Address_less_than(self):
        str_values = ("1.2.4.8", "1.2.4.9")

        values = (
            native.freelan_IPv4Address_from_string(self.ectx, str_values[0]),
            native.freelan_IPv4Address_from_string(self.ectx, str_values[1]),
        )
        self.addCleanup(native.freelan_IPv4Address_free, values[0])
        self.addCleanup(native.freelan_IPv4Address_free, values[1])

        result = native.freelan_IPv4Address_less_than(*values)
        self.assertNotEqual(0, result)

    def test_IPv4Address_equal(self):
        str_values = ("1.2.4.8", "1.2.4.9")

        values = (
            native.freelan_IPv4Address_from_string(self.ectx, str_values[0]),
            native.freelan_IPv4Address_from_string(self.ectx, str_values[1]),
        )
        self.addCleanup(native.freelan_IPv4Address_free, values[0])
        self.addCleanup(native.freelan_IPv4Address_free, values[1])

        result = native.freelan_IPv4Address_equal(*values)
        self.assertEqual(0, result)

    def test_IPv6Address_from_string_simple(self):
        result = native.freelan_IPv6Address_from_string(self.ectx, "ffe0::abcd")
        self.addCleanup(native.freelan_IPv6Address_free, result)

        self.assertNotEqual(ffi.NULL, result)

    def test_IPv6Address_from_string_incorrect_value(self):
        result = native.freelan_IPv6Address_from_string(self.ectx, "incorrect value")

        self.assertEqual(ffi.NULL, result)

    def test_IPv6Address_from_string_empty_value(self):
        result = native.freelan_IPv6Address_from_string(self.ectx, "")

        self.assertEqual(ffi.NULL, result)

    def test_IPv6Address_to_string_simple(self):
        str_value = "ffe0::abcd"

        value = native.freelan_IPv6Address_from_string(self.ectx, str_value)
        self.addCleanup(native.freelan_IPv6Address_free, value)

        result = native.freelan_IPv6Address_to_string(self.ectx, value)
        self.addCleanup(native.freelan_free, result)

        self.assertEqual(str_value, ffi.string(result))

    def test_IPv6Address_less_than(self):
        str_values = ("ffe0::abcd", "ffe0::abce")

        values = (
            native.freelan_IPv6Address_from_string(self.ectx, str_values[0]),
            native.freelan_IPv6Address_from_string(self.ectx, str_values[1]),
        )
        self.addCleanup(native.freelan_IPv6Address_free, values[0])
        self.addCleanup(native.freelan_IPv6Address_free, values[1])

        result = native.freelan_IPv6Address_less_than(*values)
        self.assertNotEqual(0, result)

    def test_IPv6Address_equal(self):
        str_values = ("ffe0::abcd", "ffe0::abce")

        values = (
            native.freelan_IPv6Address_from_string(self.ectx, str_values[0]),
            native.freelan_IPv6Address_from_string(self.ectx, str_values[1]),
        )
        self.addCleanup(native.freelan_IPv6Address_free, values[0])
        self.addCleanup(native.freelan_IPv6Address_free, values[1])

        result = native.freelan_IPv6Address_equal(*values)
        self.assertEqual(0, result)

    def test_Hostname_from_string_simple(self):
        result = native.freelan_Hostname_from_string(self.ectx, "my.host.name")
        self.addCleanup(native.freelan_Hostname_free, result)

        self.assertNotEqual(ffi.NULL, result)

    def test_Hostname_from_string_incorrect_value(self):
        result = native.freelan_Hostname_from_string(self.ectx, "incorrect value")

        self.assertEqual(ffi.NULL, result)

    def test_Hostname_from_string_empty_value(self):
        result = native.freelan_Hostname_from_string(self.ectx, "")

        self.assertEqual(ffi.NULL, result)

    def test_Hostname_to_string_simple(self):
        str_value = "my.host.name"

        value = native.freelan_Hostname_from_string(self.ectx, str_value)
        self.addCleanup(native.freelan_Hostname_free, value)

        result = native.freelan_Hostname_to_string(self.ectx, value)
        self.addCleanup(native.freelan_free, result)

        self.assertEqual(str_value, ffi.string(result))

    def test_Hostname_less_than(self):
        str_values = ("my.host.name1", "my.host.name2")

        values = (
            native.freelan_Hostname_from_string(self.ectx, str_values[0]),
            native.freelan_Hostname_from_string(self.ectx, str_values[1]),
        )
        self.addCleanup(native.freelan_Hostname_free, values[0])
        self.addCleanup(native.freelan_Hostname_free, values[1])

        result = native.freelan_Hostname_less_than(*values)
        self.assertNotEqual(0, result)

    def test_Hostname_equal(self):
        str_values = ("my.host.name1", "my.host.name2")

        values = (
            native.freelan_Hostname_from_string(self.ectx, str_values[0]),
            native.freelan_Hostname_from_string(self.ectx, str_values[1]),
        )
        self.addCleanup(native.freelan_Hostname_free, values[0])
        self.addCleanup(native.freelan_Hostname_free, values[1])

        result = native.freelan_Hostname_equal(*values)
        self.assertEqual(0, result)

    def test_PortNumber_from_string_simple(self):
        result = native.freelan_PortNumber_from_string(self.ectx, "12000")
        self.addCleanup(native.freelan_PortNumber_free, result)

        self.assertNotEqual(ffi.NULL, result)

    def test_PortNumber_from_string_incorrect_value(self):
        result = native.freelan_PortNumber_from_string(self.ectx, "incorrect value")

        self.assertEqual(ffi.NULL, result)

    def test_PortNumber_from_string_empty_value(self):
        result = native.freelan_PortNumber_from_string(self.ectx, "")

        self.assertEqual(ffi.NULL, result)

    def test_PortNumber_to_string_simple(self):
        str_value = "12000"

        value = native.freelan_PortNumber_from_string(self.ectx, str_value)
        self.addCleanup(native.freelan_PortNumber_free, value)

        result = native.freelan_PortNumber_to_string(self.ectx, value)
        self.addCleanup(native.freelan_free, result)

        self.assertEqual(str_value, ffi.string(result))

    def test_PortNumber_less_than(self):
        str_values = ("12000", "12001")

        values = (
            native.freelan_PortNumber_from_string(self.ectx, str_values[0]),
            native.freelan_PortNumber_from_string(self.ectx, str_values[1]),
        )
        self.addCleanup(native.freelan_PortNumber_free, values[0])
        self.addCleanup(native.freelan_PortNumber_free, values[1])

        result = native.freelan_PortNumber_less_than(*values)
        self.assertNotEqual(0, result)

    def test_PortNumber_equal(self):
        str_values = ("12000", "12001")

        values = (
            native.freelan_PortNumber_from_string(self.ectx, str_values[0]),
            native.freelan_PortNumber_from_string(self.ectx, str_values[1]),
        )
        self.addCleanup(native.freelan_PortNumber_free, values[0])
        self.addCleanup(native.freelan_PortNumber_free, values[1])

        result = native.freelan_PortNumber_equal(*values)
        self.assertEqual(0, result)

    def test_IPv4PrefixLength_from_string_simple(self):
        result = native.freelan_IPv4PrefixLength_from_string(self.ectx, "12")
        self.addCleanup(native.freelan_IPv4PrefixLength_free, result)

        self.assertNotEqual(ffi.NULL, result)

    def test_IPv4PrefixLength_from_string_incorrect_value(self):
        result = native.freelan_IPv4PrefixLength_from_string(self.ectx, "incorrect value")

        self.assertEqual(ffi.NULL, result)

    def test_IPv4PrefixLength_from_string_empty_value(self):
        result = native.freelan_IPv4PrefixLength_from_string(self.ectx, "")

        self.assertEqual(ffi.NULL, result)

    def test_IPv4PrefixLength_to_string_simple(self):
        str_value = "12"

        value = native.freelan_IPv4PrefixLength_from_string(self.ectx, str_value)
        self.addCleanup(native.freelan_IPv4PrefixLength_free, value)

        result = native.freelan_IPv4PrefixLength_to_string(self.ectx, value)
        self.addCleanup(native.freelan_free, result)

        self.assertEqual(str_value, ffi.string(result))

    def test_IPv4PrefixLength_less_than(self):
        str_values = ("12", "13")

        values = (
            native.freelan_IPv4PrefixLength_from_string(self.ectx, str_values[0]),
            native.freelan_IPv4PrefixLength_from_string(self.ectx, str_values[1]),
        )
        self.addCleanup(native.freelan_IPv4PrefixLength_free, values[0])
        self.addCleanup(native.freelan_IPv4PrefixLength_free, values[1])

        result = native.freelan_IPv4PrefixLength_less_than(*values)
        self.assertNotEqual(0, result)

    def test_IPv4PrefixLength_equal(self):
        str_values = ("12", "13")

        values = (
            native.freelan_IPv4PrefixLength_from_string(self.ectx, str_values[0]),
            native.freelan_IPv4PrefixLength_from_string(self.ectx, str_values[1]),
        )
        self.addCleanup(native.freelan_IPv4PrefixLength_free, values[0])
        self.addCleanup(native.freelan_IPv4PrefixLength_free, values[1])

        result = native.freelan_IPv4PrefixLength_equal(*values)
        self.assertEqual(0, result)

    def test_IPv6PrefixLength_from_string_simple(self):
        result = native.freelan_IPv6PrefixLength_from_string(self.ectx, "12")
        self.addCleanup(native.freelan_IPv6PrefixLength_free, result)

        self.assertNotEqual(ffi.NULL, result)

    def test_IPv6PrefixLength_from_string_incorrect_value(self):
        result = native.freelan_IPv6PrefixLength_from_string(self.ectx, "incorrect value")

        self.assertEqual(ffi.NULL, result)

    def test_IPv6PrefixLength_from_string_empty_value(self):
        result = native.freelan_IPv6PrefixLength_from_string(self.ectx, "")

        self.assertEqual(ffi.NULL, result)

    def test_IPv6PrefixLength_to_string_simple(self):
        str_value = "12"

        value = native.freelan_IPv6PrefixLength_from_string(self.ectx, str_value)
        self.addCleanup(native.freelan_IPv6PrefixLength_free, value)

        result = native.freelan_IPv6PrefixLength_to_string(self.ectx, value)
        self.addCleanup(native.freelan_free, result)

        self.assertEqual(str_value, ffi.string(result))

    def test_IPv6PrefixLength_less_than(self):
        str_values = ("12", "13")

        values = (
            native.freelan_IPv6PrefixLength_from_string(self.ectx, str_values[0]),
            native.freelan_IPv6PrefixLength_from_string(self.ectx, str_values[1]),
        )
        self.addCleanup(native.freelan_IPv6PrefixLength_free, values[0])
        self.addCleanup(native.freelan_IPv6PrefixLength_free, values[1])

        result = native.freelan_IPv6PrefixLength_less_than(*values)
        self.assertNotEqual(0, result)

    def test_IPv6PrefixLength_equal(self):
        str_values = ("12", "13")

        values = (
            native.freelan_IPv6PrefixLength_from_string(self.ectx, str_values[0]),
            native.freelan_IPv6PrefixLength_from_string(self.ectx, str_values[1]),
        )
        self.addCleanup(native.freelan_IPv6PrefixLength_free, values[0])
        self.addCleanup(native.freelan_IPv6PrefixLength_free, values[1])

        result = native.freelan_IPv6PrefixLength_equal(*values)
        self.assertEqual(0, result)
