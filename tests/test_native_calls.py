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

    def test_get_version_string(self):
        result = native.freelan_get_version_string()
        self.assertNotEqual(ffi.NULL, result)
        self.assertRegexpMatches(ffi.string(result), r'^\d+\.\d+\.\d+$')

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

    def test_set_log_function(self):
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

        self.addCleanup(native.freelan_set_log_function, ffi.NULL)

        native.freelan_set_log_function(callback)

    def test_log_level(self):
        level = native.FREELAN_LOG_LEVEL_DEBUG
        native.freelan_set_log_level(level)
        result = native.freelan_get_log_level()

        self.assertEqual(level, result)

    def test_log_simple(self):
        self.addCleanup(native.freelan_set_log_function, ffi.NULL)

        context = {'call_count': 0}
        p_domain = ffi.new("char[]", "mydomain")
        p_code = ffi.new("char[]", "mycode")
        p_file = ffi.new("char[]", "myfile")

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
            self.assertEqual(ffi.string(p_domain), ffi.string(domain))
            self.assertEqual(ffi.string(p_code), ffi.string(code))
            self.assertEqual(0, payload_size)
            self.assertEqual(ffi.NULL, payload)
            self.assertEqual(p_file, file)
            self.assertEqual(123, line)

            context['call_count'] += 1

            return 1

        c_callback = ffi.callback(
            "int (FreeLANLogLevel, FreeLANTimestamp, char *, char *, size_t, "
            "struct FreeLANLogPayload *, char *, unsigned int)",
        )(callback)

        native.freelan_set_log_function(c_callback)

        result = native.freelan_log(
            native.FREELAN_LOG_LEVEL_IMPORTANT,
            42,
            p_domain,
            p_code,
            0,
            ffi.NULL,
            p_file,
            123,
        )

        self.assertEqual(1, context['call_count'])
        self.assertEqual(1, result)

    def test_log_extended(self):
        self.addCleanup(native.freelan_set_log_function, ffi.NULL)

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
            self.assertEqual(ffi.string(p_domain), ffi.string(domain))
            self.assertEqual(ffi.string(p_code), ffi.string(code))
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

        native.freelan_set_log_function(callback)

        log = native.freelan_log_start(
            native.FREELAN_LOG_LEVEL_IMPORTANT,
            42,
            p_domain,
            p_code,
            p_file,
            123,
        )
        hello_str = ffi.new("const char[]", "hello")
        native.freelan_log_attach(log, "a", native.FREELAN_LOG_PAYLOAD_TYPE_STRING, {"as_string": hello_str})
        native.freelan_log_attach(log, "b", native.FREELAN_LOG_PAYLOAD_TYPE_INTEGER, {"as_integer": 42})
        native.freelan_log_attach(log, "c", native.FREELAN_LOG_PAYLOAD_TYPE_FLOAT, {"as_float": 3.14})
        native.freelan_log_attach(log, "d", native.FREELAN_LOG_PAYLOAD_TYPE_BOOLEAN, {"as_boolean": True})
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

    def test_IPv4Endpoint_from_string_simple(self):
        result = native.freelan_IPv4Endpoint_from_string(self.ectx, "9.0.0.1:12000")
        self.addCleanup(native.freelan_IPv4Endpoint_free, result)

        self.assertNotEqual(ffi.NULL, result)

    def test_IPv4Endpoint_from_string_incorrect_value(self):
        result = native.freelan_IPv4Endpoint_from_string(self.ectx, "incorrect value")

        self.assertEqual(ffi.NULL, result)

    def test_IPv4Endpoint_from_string_empty_value(self):
        result = native.freelan_IPv4Endpoint_from_string(self.ectx, "")

        self.assertEqual(ffi.NULL, result)

    def test_IPv4Endpoint_from_parts(self):
        ip_address = native.freelan_IPv4Address_from_string(self.ectx, "9.0.0.1")
        self.addCleanup(native.freelan_IPv4Address_free, ip_address)
        port_number = native.freelan_PortNumber_from_string(self.ectx, "12000")
        self.addCleanup(native.freelan_PortNumber_free, port_number)
        result = native.freelan_IPv4Endpoint_from_parts(ip_address, port_number)
        self.addCleanup(native.freelan_IPv4Endpoint_free, result)

        self.assertNotEqual(ffi.NULL, result)

    def test_IPv4Endpoint_to_string_simple(self):
        str_value = "9.0.0.1:12000"

        value = native.freelan_IPv4Endpoint_from_string(self.ectx, str_value)
        self.addCleanup(native.freelan_IPv4Endpoint_free, value)

        result = native.freelan_IPv4Endpoint_to_string(self.ectx, value)
        self.addCleanup(native.freelan_free, result)

        self.assertEqual(str_value, ffi.string(result))

    def test_IPv4Endpoint_less_than(self):
        str_values = ("9.0.0.1:12000", "9.0.0.2:11000")

        values = (
            native.freelan_IPv4Endpoint_from_string(self.ectx, str_values[0]),
            native.freelan_IPv4Endpoint_from_string(self.ectx, str_values[1]),
        )
        self.addCleanup(native.freelan_IPv4Endpoint_free, values[0])
        self.addCleanup(native.freelan_IPv4Endpoint_free, values[1])

        result = native.freelan_IPv4Endpoint_less_than(*values)
        self.assertNotEqual(0, result)

    def test_IPv4Endpoint_equal(self):
        str_values = ("9.0.0.1:12000", "9.0.0.2:11000")

        values = (
            native.freelan_IPv4Endpoint_from_string(self.ectx, str_values[0]),
            native.freelan_IPv4Endpoint_from_string(self.ectx, str_values[1]),
        )
        self.addCleanup(native.freelan_IPv4Endpoint_free, values[0])
        self.addCleanup(native.freelan_IPv4Endpoint_free, values[1])

        result = native.freelan_IPv4Endpoint_equal(*values)
        self.assertEqual(0, result)

    def test_IPv4Endpoint_get_IPv4Address(self):
        endpoint = native.freelan_IPv4Endpoint_from_string(self.ectx, "9.0.0.1:12000")
        self.addCleanup(native.freelan_IPv4Endpoint_free, endpoint)
        result = native.freelan_IPv4Endpoint_get_IPv4Address(endpoint)
        self.addCleanup(native.freelan_IPv4Address_free, result)

        self.assertNotEqual(ffi.NULL, result)

    def test_IPv4Endpoint_get_PortNumber(self):
        endpoint = native.freelan_IPv4Endpoint_from_string(self.ectx, "9.0.0.1:12000")
        self.addCleanup(native.freelan_IPv4Endpoint_free, endpoint)
        result = native.freelan_IPv4Endpoint_get_PortNumber(endpoint)
        self.addCleanup(native.freelan_PortNumber_free, result)

        self.assertNotEqual(ffi.NULL, result)

    def test_IPv6Endpoint_from_string_simple(self):
        result = native.freelan_IPv6Endpoint_from_string(self.ectx, "[fe80::a:1]:12000")
        self.addCleanup(native.freelan_IPv6Endpoint_free, result)

        self.assertNotEqual(ffi.NULL, result)

    def test_IPv6Endpoint_from_string_incorrect_value(self):
        result = native.freelan_IPv6Endpoint_from_string(self.ectx, "incorrect value")

        self.assertEqual(ffi.NULL, result)

    def test_IPv6Endpoint_from_string_empty_value(self):
        result = native.freelan_IPv6Endpoint_from_string(self.ectx, "")

        self.assertEqual(ffi.NULL, result)

    def test_IPv6Endpoint_from_parts(self):
        ip_address = native.freelan_IPv6Address_from_string(self.ectx, "fe80::a:1")
        self.addCleanup(native.freelan_IPv6Address_free, ip_address)
        port_number = native.freelan_PortNumber_from_string(self.ectx, "12000")
        self.addCleanup(native.freelan_PortNumber_free, port_number)
        result = native.freelan_IPv6Endpoint_from_parts(ip_address, port_number)
        self.addCleanup(native.freelan_IPv6Endpoint_free, result)

        self.assertNotEqual(ffi.NULL, result)

    def test_IPv6Endpoint_to_string_simple(self):
        str_value = "[fe80::a:1]:12000"

        value = native.freelan_IPv6Endpoint_from_string(self.ectx, str_value)
        self.addCleanup(native.freelan_IPv6Endpoint_free, value)

        result = native.freelan_IPv6Endpoint_to_string(self.ectx, value)
        self.addCleanup(native.freelan_free, result)

        self.assertEqual(str_value, ffi.string(result))

    def test_IPv6Endpoint_less_than(self):
        str_values = ("[fe80::a:1]:12000", "[fe80::a:2]:11000")

        values = (
            native.freelan_IPv6Endpoint_from_string(self.ectx, str_values[0]),
            native.freelan_IPv6Endpoint_from_string(self.ectx, str_values[1]),
        )
        self.addCleanup(native.freelan_IPv6Endpoint_free, values[0])
        self.addCleanup(native.freelan_IPv6Endpoint_free, values[1])

        result = native.freelan_IPv6Endpoint_less_than(*values)
        self.assertNotEqual(0, result)

    def test_IPv6Endpoint_equal(self):
        str_values = ("[fe80::a:1]:12000", "[fe80::a:2]:11000")

        values = (
            native.freelan_IPv6Endpoint_from_string(self.ectx, str_values[0]),
            native.freelan_IPv6Endpoint_from_string(self.ectx, str_values[1]),
        )
        self.addCleanup(native.freelan_IPv6Endpoint_free, values[0])
        self.addCleanup(native.freelan_IPv6Endpoint_free, values[1])

        result = native.freelan_IPv6Endpoint_equal(*values)
        self.assertEqual(0, result)

    def test_IPv6Endpoint_get_IPv6Address(self):
        endpoint = native.freelan_IPv6Endpoint_from_string(self.ectx, "[fe80::a:1]:12000")
        self.addCleanup(native.freelan_IPv6Endpoint_free, endpoint)
        result = native.freelan_IPv6Endpoint_get_IPv6Address(endpoint)
        self.addCleanup(native.freelan_IPv6Address_free, result)

        self.assertNotEqual(ffi.NULL, result)

    def test_IPv6Endpoint_get_PortNumber(self):
        endpoint = native.freelan_IPv6Endpoint_from_string(self.ectx, "[fe80::a:1]:12000")
        self.addCleanup(native.freelan_IPv6Endpoint_free, endpoint)
        result = native.freelan_IPv6Endpoint_get_PortNumber(endpoint)
        self.addCleanup(native.freelan_PortNumber_free, result)

        self.assertNotEqual(ffi.NULL, result)

    def test_HostnameEndpoint_from_string_simple(self):
        result = native.freelan_HostnameEndpoint_from_string(self.ectx, "foo.bar:12000")
        self.addCleanup(native.freelan_HostnameEndpoint_free, result)

        self.assertNotEqual(ffi.NULL, result)

    def test_HostnameEndpoint_from_string_incorrect_value(self):
        result = native.freelan_HostnameEndpoint_from_string(self.ectx, "incorrect value")

        self.assertEqual(ffi.NULL, result)

    def test_HostnameEndpoint_from_string_empty_value(self):
        result = native.freelan_HostnameEndpoint_from_string(self.ectx, "")

        self.assertEqual(ffi.NULL, result)

    def test_HostnameEndpoint_from_parts(self):
        hostname = native.freelan_Hostname_from_string(self.ectx, "foo.bar")
        self.addCleanup(native.freelan_Hostname_free, hostname)
        port_number = native.freelan_PortNumber_from_string(self.ectx, "12000")
        self.addCleanup(native.freelan_PortNumber_free, port_number)
        result = native.freelan_HostnameEndpoint_from_parts(hostname, port_number)
        self.addCleanup(native.freelan_HostnameEndpoint_free, result)

        self.assertNotEqual(ffi.NULL, result)

    def test_HostnameEndpoint_to_string_simple(self):
        str_value = "foo.bar:12000"

        value = native.freelan_HostnameEndpoint_from_string(self.ectx, str_value)
        self.addCleanup(native.freelan_HostnameEndpoint_free, value)

        result = native.freelan_HostnameEndpoint_to_string(self.ectx, value)
        self.addCleanup(native.freelan_free, result)

        self.assertEqual(str_value, ffi.string(result))

    def test_HostnameEndpoint_less_than(self):
        str_values = ("foo.bar:12000", "foo.baz:11000")

        values = (
            native.freelan_HostnameEndpoint_from_string(self.ectx, str_values[0]),
            native.freelan_HostnameEndpoint_from_string(self.ectx, str_values[1]),
        )
        self.addCleanup(native.freelan_HostnameEndpoint_free, values[0])
        self.addCleanup(native.freelan_HostnameEndpoint_free, values[1])

        result = native.freelan_HostnameEndpoint_less_than(*values)
        self.assertNotEqual(0, result)

    def test_HostnameEndpoint_equal(self):
        str_values = ("foo.bar:12000", "foo.baz:11000")

        values = (
            native.freelan_HostnameEndpoint_from_string(self.ectx, str_values[0]),
            native.freelan_HostnameEndpoint_from_string(self.ectx, str_values[1]),
        )
        self.addCleanup(native.freelan_HostnameEndpoint_free, values[0])
        self.addCleanup(native.freelan_HostnameEndpoint_free, values[1])

        result = native.freelan_HostnameEndpoint_equal(*values)
        self.assertEqual(0, result)

    def test_HostnameEndpoint_get_Hostname(self):
        endpoint = native.freelan_HostnameEndpoint_from_string(self.ectx, "foo.bar:12000")
        self.addCleanup(native.freelan_HostnameEndpoint_free, endpoint)
        result = native.freelan_HostnameEndpoint_get_Hostname(endpoint)
        self.addCleanup(native.freelan_Hostname_free, result)

        self.assertNotEqual(ffi.NULL, result)

    def test_HostnameEndpoint_get_PortNumber(self):
        endpoint = native.freelan_HostnameEndpoint_from_string(self.ectx, "foo.bar:12000")
        self.addCleanup(native.freelan_HostnameEndpoint_free, endpoint)
        result = native.freelan_HostnameEndpoint_get_PortNumber(endpoint)
        self.addCleanup(native.freelan_PortNumber_free, result)

        self.assertNotEqual(ffi.NULL, result)

    def test_IPv4Route_from_string_simple(self):
        result = native.freelan_IPv4Route_from_string(self.ectx, "9.0.0.1/24")
        self.addCleanup(native.freelan_IPv4Route_free, result)

        self.assertNotEqual(ffi.NULL, result)

    def test_IPv4Route_from_string_incorrect_value(self):
        result = native.freelan_IPv4Route_from_string(self.ectx, "incorrect value")

        self.assertEqual(ffi.NULL, result)

    def test_IPv4Route_from_string_empty_value(self):
        result = native.freelan_IPv4Route_from_string(self.ectx, "")

        self.assertEqual(ffi.NULL, result)

    def test_IPv4Route_from_parts(self):
        ip_address = native.freelan_IPv4Address_from_string(self.ectx, "9.0.0.1")
        self.addCleanup(native.freelan_IPv4Address_free, ip_address)
        prefix_length = native.freelan_IPv4PrefixLength_from_string(self.ectx, "24")
        self.addCleanup(native.freelan_IPv4PrefixLength_free, prefix_length)
        result = native.freelan_IPv4Route_from_parts(ip_address, prefix_length)
        self.addCleanup(native.freelan_IPv4Route_free, result)

        self.assertNotEqual(ffi.NULL, result)

    def test_IPv4Route_to_string_simple(self):
        str_value = "9.0.0.0/24"

        value = native.freelan_IPv4Route_from_string(self.ectx, str_value)
        self.addCleanup(native.freelan_IPv4Route_free, value)

        result = native.freelan_IPv4Route_to_string(self.ectx, value)
        self.addCleanup(native.freelan_free, result)

        self.assertEqual(str_value, ffi.string(result))

    def test_IPv4Route_less_than(self):
        str_values = ("9.0.0.0/24", "10.0.0.0/24")

        values = (
            native.freelan_IPv4Route_from_string(self.ectx, str_values[0]),
            native.freelan_IPv4Route_from_string(self.ectx, str_values[1]),
        )
        self.addCleanup(native.freelan_IPv4Route_free, values[0])
        self.addCleanup(native.freelan_IPv4Route_free, values[1])

        result = native.freelan_IPv4Route_less_than(*values)
        self.assertNotEqual(0, result)

    def test_IPv4Route_equal(self):
        str_values = ("9.0.0.0/24", "10.0.0.0/24")

        values = (
            native.freelan_IPv4Route_from_string(self.ectx, str_values[0]),
            native.freelan_IPv4Route_from_string(self.ectx, str_values[1]),
        )
        self.addCleanup(native.freelan_IPv4Route_free, values[0])
        self.addCleanup(native.freelan_IPv4Route_free, values[1])

        result = native.freelan_IPv4Route_equal(*values)
        self.assertEqual(0, result)

    def test_IPv4Route_get_IPv4Address(self):
        endpoint = native.freelan_IPv4Route_from_string(self.ectx, "9.0.0.0/24")
        self.addCleanup(native.freelan_IPv4Route_free, endpoint)
        result = native.freelan_IPv4Route_get_IPv4Address(endpoint)
        self.addCleanup(native.freelan_IPv4Address_free, result)

        self.assertNotEqual(ffi.NULL, result)

    def test_IPv4Route_get_IPv4PrefixLength(self):
        endpoint = native.freelan_IPv4Route_from_string(self.ectx, "9.0.0.0/24")
        self.addCleanup(native.freelan_IPv4Route_free, endpoint)
        result = native.freelan_IPv4Route_get_IPv4PrefixLength(endpoint)
        self.addCleanup(native.freelan_IPv4PrefixLength_free, result)

        self.assertNotEqual(ffi.NULL, result)

    def test_IPv6Route_from_string_simple(self):
        result = native.freelan_IPv6Route_from_string(self.ectx, "fe80::a:0/120")
        self.addCleanup(native.freelan_IPv6Route_free, result)

        self.assertNotEqual(ffi.NULL, result)

    def test_IPv6Route_from_string_incorrect_value(self):
        result = native.freelan_IPv6Route_from_string(self.ectx, "incorrect value")

        self.assertEqual(ffi.NULL, result)

    def test_IPv6Route_from_string_empty_value(self):
        result = native.freelan_IPv6Route_from_string(self.ectx, "")

        self.assertEqual(ffi.NULL, result)

    def test_IPv6Route_from_parts(self):
        ip_address = native.freelan_IPv6Address_from_string(self.ectx, "fe80::a:0")
        self.addCleanup(native.freelan_IPv6Address_free, ip_address)
        prefix_length = native.freelan_IPv6PrefixLength_from_string(self.ectx, "120")
        self.addCleanup(native.freelan_IPv6PrefixLength_free, prefix_length)
        result = native.freelan_IPv6Route_from_parts(ip_address, prefix_length)
        self.addCleanup(native.freelan_IPv6Route_free, result)

        self.assertNotEqual(ffi.NULL, result)

    def test_IPv6Route_to_string_simple(self):
        str_value = "fe80::a:0/120"

        value = native.freelan_IPv6Route_from_string(self.ectx, str_value)
        self.addCleanup(native.freelan_IPv6Route_free, value)

        result = native.freelan_IPv6Route_to_string(self.ectx, value)
        self.addCleanup(native.freelan_free, result)

        self.assertEqual(str_value, ffi.string(result))

    def test_IPv6Route_less_than(self):
        str_values = ("fe80::a:0/120", "fe90::b:0/120")

        values = (
            native.freelan_IPv6Route_from_string(self.ectx, str_values[0]),
            native.freelan_IPv6Route_from_string(self.ectx, str_values[1]),
        )
        self.addCleanup(native.freelan_IPv6Route_free, values[0])
        self.addCleanup(native.freelan_IPv6Route_free, values[1])

        result = native.freelan_IPv6Route_less_than(*values)
        self.assertNotEqual(0, result)

    def test_IPv6Route_equal(self):
        str_values = ("fe80::a:0/120", "fe90::a:0/120")

        values = (
            native.freelan_IPv6Route_from_string(self.ectx, str_values[0]),
            native.freelan_IPv6Route_from_string(self.ectx, str_values[1]),
        )
        self.addCleanup(native.freelan_IPv6Route_free, values[0])
        self.addCleanup(native.freelan_IPv6Route_free, values[1])

        result = native.freelan_IPv6Route_equal(*values)
        self.assertEqual(0, result)

    def test_IPv6Route_get_IPv6Address(self):
        endpoint = native.freelan_IPv6Route_from_string(self.ectx, "fe80::a:0/120")
        self.addCleanup(native.freelan_IPv6Route_free, endpoint)
        result = native.freelan_IPv6Route_get_IPv6Address(endpoint)
        self.addCleanup(native.freelan_IPv6Address_free, result)

        self.assertNotEqual(ffi.NULL, result)

    def test_IPv6Route_get_IPv6PrefixLength(self):
        endpoint = native.freelan_IPv6Route_from_string(self.ectx, "fe80::a:0/120")
        self.addCleanup(native.freelan_IPv6Route_free, endpoint)
        result = native.freelan_IPv6Route_get_IPv6PrefixLength(endpoint)
        self.addCleanup(native.freelan_IPv6PrefixLength_free, result)

        self.assertNotEqual(ffi.NULL, result)
