"""
Test direct calling of the C methods.
"""

from __future__ import unicode_literals

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
        result = native.freelan_strdup(b"freelan")
        self.addCleanup(native.freelan_free, result)

        self.assertEqual(b"freelan", ffi.string(result))

    def test_get_version_string(self):
        result = native.freelan_get_version_string()
        self.assertNotEqual(ffi.NULL, result)
        self.assertRegexpMatches(ffi.string(result), br'^\d+\.\d+\.\d+$')

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
            "int (freelan_LogLevel, freelan_Timestamp, char *, char *, size_t,"
            " struct freelan_LogPayload *, char *, unsigned int)",
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
        p_domain = ffi.new("char[]", b"mydomain")
        p_code = ffi.new("char[]", b"mycode")
        p_file = ffi.new("char[]", b"myfile")

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
            "int (freelan_LogLevel, freelan_Timestamp, char *, char *, size_t,"
            " struct freelan_LogPayload *, char *, unsigned int)",
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
        p_domain = ffi.new("char[]", b"mydomain")
        p_code = ffi.new("char[]", b"mycode")
        p_file = ffi.new("char[]", b"myfile")

        @ffi.callback(
            "int (freelan_LogLevel, freelan_Timestamp, char *, char *, size_t,"
            " struct freelan_LogPayload *, char *, unsigned int)",
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

            self.assertEqual(b"a", ffi.string(payload[0].key))
            self.assertEqual(
                native.FREELAN_LOG_PAYLOAD_TYPE_STRING,
                payload[0].type,
            )
            self.assertEqual(b"hello", ffi.string(payload[0].value.as_string))

            self.assertEqual(b"b", ffi.string(payload[1].key))
            self.assertEqual(
                native.FREELAN_LOG_PAYLOAD_TYPE_INTEGER,
                payload[1].type,
            )
            self.assertEqual(42, payload[1].value.as_integer)

            self.assertEqual(b"c", ffi.string(payload[2].key))
            self.assertEqual(
                native.FREELAN_LOG_PAYLOAD_TYPE_FLOAT,
                payload[2].type,
            )
            self.assertAlmostEqual(3.14, payload[2].value.as_float)

            self.assertEqual(b"d", ffi.string(payload[3].key))
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
        hello_str = ffi.new("const char[]", b"hello")
        native.freelan_log_attach(
            log,
            b"a",
            native.FREELAN_LOG_PAYLOAD_TYPE_STRING,
            {"as_string": hello_str},
        )
        native.freelan_log_attach(
            log,
            b"b",
            native.FREELAN_LOG_PAYLOAD_TYPE_INTEGER,
            {"as_integer": 42},
        )
        native.freelan_log_attach(
            log,
            b"c",
            native.FREELAN_LOG_PAYLOAD_TYPE_FLOAT,
            {"as_float": 3.14},
        )
        native.freelan_log_attach(
            log,
            b"d",
            native.FREELAN_LOG_PAYLOAD_TYPE_BOOLEAN,
            {"as_boolean": True},
        )
        result = native.freelan_log_complete(log)

        self.assertEqual(1, context['call_count'])
        self.assertEqual(1, result)

    def test_EthernetAddress_from_string_simple(self):
        result = native.freelan_EthernetAddress_from_string(
            self.ectx,
            b"ab:cd:ef:12:34:56",
        )
        self.addCleanup(native.freelan_EthernetAddress_free, result)

        self.assertNotEqual(ffi.NULL, result)

    def test_EthernetAddress_from_string_truncated(self):
        result = native.freelan_EthernetAddress_from_string(
            self.ectx,
            b"ab:cd:ef:12:",
        )

        self.assertEqual(ffi.NULL, result)

    def test_EthernetAddress_from_string_incorrect_value(self):
        result = native.freelan_EthernetAddress_from_string(
            self.ectx,
            b"incorrect value",
        )

        self.assertEqual(ffi.NULL, result)

    def test_EthernetAddress_from_string_empty_value(self):
        result = native.freelan_EthernetAddress_from_string(self.ectx, b"")

        self.assertEqual(ffi.NULL, result)

    def test_EthernetAddress_to_string_simple(self):
        str_value = b"ab:cd:ef:12:34:56"

        value = native.freelan_EthernetAddress_from_string(
            self.ectx,
            str_value,
        )
        self.addCleanup(native.freelan_EthernetAddress_free, value)

        result = native.freelan_EthernetAddress_to_string(self.ectx, value)
        self.addCleanup(native.freelan_free, result)

        self.assertEqual(str_value, ffi.string(result))

    def test_EthernetAddress_clone(self):
        inst = native.freelan_EthernetAddress_from_string(
            self.ectx,
            b"ab:cd:ef:12:34:56",
        )
        self.addCleanup(native.freelan_EthernetAddress_free, inst)

        result = native.freelan_EthernetAddress_clone(self.ectx, inst)
        self.addCleanup(native.freelan_EthernetAddress_free, result)

        self.assertNotEqual(ffi.NULL, result)

    def test_EthernetAddress_less_than(self):
        str_values = (b"ab:cd:ef:12:34:56", b"ab:cd:ef:12:34:57")

        values = (
            native.freelan_EthernetAddress_from_string(
                self.ectx,
                str_values[0],
            ),
            native.freelan_EthernetAddress_from_string(
                self.ectx,
                str_values[1],
            ),
        )
        self.addCleanup(native.freelan_EthernetAddress_free, values[0])
        self.addCleanup(native.freelan_EthernetAddress_free, values[1])

        result = native.freelan_EthernetAddress_less_than(*values)
        self.assertNotEqual(0, result)

    def test_EthernetAddress_equal(self):
        str_values = (b"ab:cd:ef:12:34:56", b"ab:cd:ef:12:34:57")

        values = (
            native.freelan_EthernetAddress_from_string(
                self.ectx,
                str_values[0],
            ),
            native.freelan_EthernetAddress_from_string(
                self.ectx,
                str_values[1],
            ),
        )
        self.addCleanup(native.freelan_EthernetAddress_free, values[0])
        self.addCleanup(native.freelan_EthernetAddress_free, values[1])

        result = native.freelan_EthernetAddress_equal(*values)
        self.assertEqual(0, result)

    def test_IPv4Address_from_string_simple(self):
        result = native.freelan_IPv4Address_from_string(self.ectx, b"1.2.4.8")
        self.addCleanup(native.freelan_IPv4Address_free, result)

        self.assertNotEqual(ffi.NULL, result)

    def test_IPv4Address_from_string_truncated(self):
        result = native.freelan_IPv4Address_from_string(self.ectx, b"127.1")

        self.assertEqual(ffi.NULL, result)

    def test_IPv4Address_from_string_incorrect_value(self):
        result = native.freelan_IPv4Address_from_string(
            self.ectx,
            b"incorrect value",
        )

        self.assertEqual(ffi.NULL, result)

    def test_IPv4Address_from_string_empty_value(self):
        result = native.freelan_IPv4Address_from_string(self.ectx, b"")

        self.assertEqual(ffi.NULL, result)

    def test_IPv4Address_to_string_simple(self):
        str_value = b"1.2.4.8"

        value = native.freelan_IPv4Address_from_string(self.ectx, str_value)
        self.addCleanup(native.freelan_IPv4Address_free, value)

        result = native.freelan_IPv4Address_to_string(self.ectx, value)
        self.addCleanup(native.freelan_free, result)

        self.assertEqual(str_value, ffi.string(result))

    def test_IPv4Address_clone(self):
        inst = native.freelan_IPv4Address_from_string(self.ectx, b"1.2.4.8")
        self.addCleanup(native.freelan_IPv4Address_free, inst)

        result = native.freelan_IPv4Address_clone(self.ectx, inst)
        self.addCleanup(native.freelan_IPv4Address_free, result)

        self.assertNotEqual(ffi.NULL, result)

    def test_IPv4Address_less_than(self):
        str_values = (b"1.2.4.8", b"1.2.4.9")

        values = (
            native.freelan_IPv4Address_from_string(self.ectx, str_values[0]),
            native.freelan_IPv4Address_from_string(self.ectx, str_values[1]),
        )
        self.addCleanup(native.freelan_IPv4Address_free, values[0])
        self.addCleanup(native.freelan_IPv4Address_free, values[1])

        result = native.freelan_IPv4Address_less_than(*values)
        self.assertNotEqual(0, result)

    def test_IPv4Address_equal(self):
        str_values = (b"1.2.4.8", b"1.2.4.9")

        values = (
            native.freelan_IPv4Address_from_string(self.ectx, str_values[0]),
            native.freelan_IPv4Address_from_string(self.ectx, str_values[1]),
        )
        self.addCleanup(native.freelan_IPv4Address_free, values[0])
        self.addCleanup(native.freelan_IPv4Address_free, values[1])

        result = native.freelan_IPv4Address_equal(*values)
        self.assertEqual(0, result)

    def test_IPv6Address_from_string_simple(self):
        result = native.freelan_IPv6Address_from_string(
            self.ectx,
            b"ffe0::abcd",
        )
        self.addCleanup(native.freelan_IPv6Address_free, result)

        self.assertNotEqual(ffi.NULL, result)

    def test_IPv6Address_from_string_incorrect_value(self):
        result = native.freelan_IPv6Address_from_string(
            self.ectx,
            b"incorrect value",
        )

        self.assertEqual(ffi.NULL, result)

    def test_IPv6Address_from_string_empty_value(self):
        result = native.freelan_IPv6Address_from_string(self.ectx, b"")

        self.assertEqual(ffi.NULL, result)

    def test_IPv6Address_to_string_simple(self):
        str_value = b"ffe0::abcd"

        value = native.freelan_IPv6Address_from_string(self.ectx, str_value)
        self.addCleanup(native.freelan_IPv6Address_free, value)

        result = native.freelan_IPv6Address_to_string(self.ectx, value)
        self.addCleanup(native.freelan_free, result)

        self.assertEqual(str_value, ffi.string(result))

    def test_IPv6Address_clone(self):
        inst = native.freelan_IPv6Address_from_string(self.ectx, b"fe80::a:1")
        self.addCleanup(native.freelan_IPv6Address_free, inst)

        result = native.freelan_IPv6Address_clone(self.ectx, inst)
        self.addCleanup(native.freelan_IPv6Address_free, result)

        self.assertNotEqual(ffi.NULL, result)

    def test_IPv6Address_less_than(self):
        str_values = (b"ffe0::abcd", b"ffe0::abce")

        values = (
            native.freelan_IPv6Address_from_string(self.ectx, str_values[0]),
            native.freelan_IPv6Address_from_string(self.ectx, str_values[1]),
        )
        self.addCleanup(native.freelan_IPv6Address_free, values[0])
        self.addCleanup(native.freelan_IPv6Address_free, values[1])

        result = native.freelan_IPv6Address_less_than(*values)
        self.assertNotEqual(0, result)

    def test_IPv6Address_equal(self):
        str_values = (b"ffe0::abcd", b"ffe0::abce")

        values = (
            native.freelan_IPv6Address_from_string(self.ectx, str_values[0]),
            native.freelan_IPv6Address_from_string(self.ectx, str_values[1]),
        )
        self.addCleanup(native.freelan_IPv6Address_free, values[0])
        self.addCleanup(native.freelan_IPv6Address_free, values[1])

        result = native.freelan_IPv6Address_equal(*values)
        self.assertEqual(0, result)

    def test_Hostname_from_string_simple(self):
        result = native.freelan_Hostname_from_string(
            self.ectx,
            b"my.host.name",
        )
        self.addCleanup(native.freelan_Hostname_free, result)

        self.assertNotEqual(ffi.NULL, result)

    def test_Hostname_from_string_incorrect_value(self):
        result = native.freelan_Hostname_from_string(
            self.ectx,
            b"incorrect value",
        )

        self.assertEqual(ffi.NULL, result)

    def test_Hostname_from_string_empty_value(self):
        result = native.freelan_Hostname_from_string(self.ectx, b"")

        self.assertEqual(ffi.NULL, result)

    def test_Hostname_to_string_simple(self):
        str_value = b"my.host.name"

        value = native.freelan_Hostname_from_string(self.ectx, str_value)
        self.addCleanup(native.freelan_Hostname_free, value)

        result = native.freelan_Hostname_to_string(self.ectx, value)
        self.addCleanup(native.freelan_free, result)

        self.assertEqual(str_value, ffi.string(result))

    def test_Hostname_clone(self):
        inst = native.freelan_Hostname_from_string(self.ectx, b"some.hostname")
        self.addCleanup(native.freelan_Hostname_free, inst)

        result = native.freelan_Hostname_clone(self.ectx, inst)
        self.addCleanup(native.freelan_Hostname_free, result)

        self.assertNotEqual(ffi.NULL, result)

    def test_Hostname_less_than(self):
        str_values = (b"my.host.name1", b"my.host.name2")

        values = (
            native.freelan_Hostname_from_string(self.ectx, str_values[0]),
            native.freelan_Hostname_from_string(self.ectx, str_values[1]),
        )
        self.addCleanup(native.freelan_Hostname_free, values[0])
        self.addCleanup(native.freelan_Hostname_free, values[1])

        result = native.freelan_Hostname_less_than(*values)
        self.assertNotEqual(0, result)

    def test_Hostname_equal(self):
        str_values = (b"my.host.name1", b"my.host.name2")

        values = (
            native.freelan_Hostname_from_string(self.ectx, str_values[0]),
            native.freelan_Hostname_from_string(self.ectx, str_values[1]),
        )
        self.addCleanup(native.freelan_Hostname_free, values[0])
        self.addCleanup(native.freelan_Hostname_free, values[1])

        result = native.freelan_Hostname_equal(*values)
        self.assertEqual(0, result)

    def test_PortNumber_from_string_simple(self):
        result = native.freelan_PortNumber_from_string(self.ectx, b"12000")
        self.addCleanup(native.freelan_PortNumber_free, result)

        self.assertNotEqual(ffi.NULL, result)

    def test_PortNumber_from_string_incorrect_value(self):
        result = native.freelan_PortNumber_from_string(
            self.ectx,
            b"incorrect value",
        )

        self.assertEqual(ffi.NULL, result)

    def test_PortNumber_from_string_empty_value(self):
        result = native.freelan_PortNumber_from_string(self.ectx, b"")

        self.assertEqual(ffi.NULL, result)

    def test_PortNumber_to_string_simple(self):
        str_value = b"12000"

        value = native.freelan_PortNumber_from_string(self.ectx, str_value)
        self.addCleanup(native.freelan_PortNumber_free, value)

        result = native.freelan_PortNumber_to_string(self.ectx, value)
        self.addCleanup(native.freelan_free, result)

        self.assertEqual(str_value, ffi.string(result))

    def test_PortNumber_clone(self):
        inst = native.freelan_PortNumber_from_string(self.ectx, b"12000")
        self.addCleanup(native.freelan_PortNumber_free, inst)

        result = native.freelan_PortNumber_clone(self.ectx, inst)
        self.addCleanup(native.freelan_PortNumber_free, result)

        self.assertNotEqual(ffi.NULL, result)

    def test_PortNumber_less_than(self):
        str_values = (b"12000", b"12001")

        values = (
            native.freelan_PortNumber_from_string(self.ectx, str_values[0]),
            native.freelan_PortNumber_from_string(self.ectx, str_values[1]),
        )
        self.addCleanup(native.freelan_PortNumber_free, values[0])
        self.addCleanup(native.freelan_PortNumber_free, values[1])

        result = native.freelan_PortNumber_less_than(*values)
        self.assertNotEqual(0, result)

    def test_PortNumber_equal(self):
        str_values = (b"12000", b"12001")

        values = (
            native.freelan_PortNumber_from_string(self.ectx, str_values[0]),
            native.freelan_PortNumber_from_string(self.ectx, str_values[1]),
        )
        self.addCleanup(native.freelan_PortNumber_free, values[0])
        self.addCleanup(native.freelan_PortNumber_free, values[1])

        result = native.freelan_PortNumber_equal(*values)
        self.assertEqual(0, result)

    def test_IPv4PrefixLength_from_string_simple(self):
        result = native.freelan_IPv4PrefixLength_from_string(self.ectx, b"12")
        self.addCleanup(native.freelan_IPv4PrefixLength_free, result)

        self.assertNotEqual(ffi.NULL, result)

    def test_IPv4PrefixLength_from_string_incorrect_value(self):
        result = native.freelan_IPv4PrefixLength_from_string(
            self.ectx,
            b"incorrect value",
        )

        self.assertEqual(ffi.NULL, result)

    def test_IPv4PrefixLength_from_string_empty_value(self):
        result = native.freelan_IPv4PrefixLength_from_string(self.ectx, b"")

        self.assertEqual(ffi.NULL, result)

    def test_IPv4PrefixLength_to_string_simple(self):
        str_value = b"12"

        value = native.freelan_IPv4PrefixLength_from_string(
            self.ectx,
            str_value,
        )
        self.addCleanup(native.freelan_IPv4PrefixLength_free, value)

        result = native.freelan_IPv4PrefixLength_to_string(self.ectx, value)
        self.addCleanup(native.freelan_free, result)

        self.assertEqual(str_value, ffi.string(result))

    def test_IPv4PrefixLength_clone(self):
        inst = native.freelan_IPv4PrefixLength_from_string(self.ectx, b"24")
        self.addCleanup(native.freelan_IPv4PrefixLength_free, inst)

        result = native.freelan_IPv4PrefixLength_clone(self.ectx, inst)
        self.addCleanup(native.freelan_IPv4PrefixLength_free, result)

        self.assertNotEqual(ffi.NULL, result)

    def test_IPv4PrefixLength_less_than(self):
        str_values = (b"12", b"13")

        values = (
            native.freelan_IPv4PrefixLength_from_string(
                self.ectx,
                str_values[0],
            ),
            native.freelan_IPv4PrefixLength_from_string(
                self.ectx,
                str_values[1],
            ),
        )
        self.addCleanup(native.freelan_IPv4PrefixLength_free, values[0])
        self.addCleanup(native.freelan_IPv4PrefixLength_free, values[1])

        result = native.freelan_IPv4PrefixLength_less_than(*values)
        self.assertNotEqual(0, result)

    def test_IPv4PrefixLength_equal(self):
        str_values = (b"12", b"13")

        values = (
            native.freelan_IPv4PrefixLength_from_string(
                self.ectx,
                str_values[0],
            ),
            native.freelan_IPv4PrefixLength_from_string(
                self.ectx,
                str_values[1],
            ),
        )
        self.addCleanup(native.freelan_IPv4PrefixLength_free, values[0])
        self.addCleanup(native.freelan_IPv4PrefixLength_free, values[1])

        result = native.freelan_IPv4PrefixLength_equal(*values)
        self.assertEqual(0, result)

    def test_IPv6PrefixLength_from_string_simple(self):
        result = native.freelan_IPv6PrefixLength_from_string(self.ectx, b"12")
        self.addCleanup(native.freelan_IPv6PrefixLength_free, result)

        self.assertNotEqual(ffi.NULL, result)

    def test_IPv6PrefixLength_from_string_incorrect_value(self):
        result = native.freelan_IPv6PrefixLength_from_string(
            self.ectx,
            b"incorrect value",
        )

        self.assertEqual(ffi.NULL, result)

    def test_IPv6PrefixLength_from_string_empty_value(self):
        result = native.freelan_IPv6PrefixLength_from_string(self.ectx, b"")

        self.assertEqual(ffi.NULL, result)

    def test_IPv6PrefixLength_to_string_simple(self):
        str_value = b"12"

        value = native.freelan_IPv6PrefixLength_from_string(
            self.ectx,
            str_value,
        )
        self.addCleanup(native.freelan_IPv6PrefixLength_free, value)

        result = native.freelan_IPv6PrefixLength_to_string(self.ectx, value)
        self.addCleanup(native.freelan_free, result)

        self.assertEqual(str_value, ffi.string(result))

    def test_IPv6PrefixLength_clone(self):
        inst = native.freelan_IPv6PrefixLength_from_string(self.ectx, b"64")
        self.addCleanup(native.freelan_IPv6PrefixLength_free, inst)

        result = native.freelan_IPv6PrefixLength_clone(self.ectx, inst)
        self.addCleanup(native.freelan_IPv6PrefixLength_free, result)

        self.assertNotEqual(ffi.NULL, result)

    def test_IPv6PrefixLength_less_than(self):
        str_values = (b"12", b"13")

        values = (
            native.freelan_IPv6PrefixLength_from_string(
                self.ectx,
                str_values[0],
            ),
            native.freelan_IPv6PrefixLength_from_string(
                self.ectx,
                str_values[1],
            ),
        )
        self.addCleanup(native.freelan_IPv6PrefixLength_free, values[0])
        self.addCleanup(native.freelan_IPv6PrefixLength_free, values[1])

        result = native.freelan_IPv6PrefixLength_less_than(*values)
        self.assertNotEqual(0, result)

    def test_IPv6PrefixLength_equal(self):
        str_values = (b"12", b"13")

        values = (
            native.freelan_IPv6PrefixLength_from_string(
                self.ectx,
                str_values[0],
            ),
            native.freelan_IPv6PrefixLength_from_string(
                self.ectx,
                str_values[1],
            ),
        )
        self.addCleanup(native.freelan_IPv6PrefixLength_free, values[0])
        self.addCleanup(native.freelan_IPv6PrefixLength_free, values[1])

        result = native.freelan_IPv6PrefixLength_equal(*values)
        self.assertEqual(0, result)

    def test_IPv4Endpoint_from_string_simple(self):
        result = native.freelan_IPv4Endpoint_from_string(
            self.ectx,
            b"9.0.0.1:12000",
        )
        self.addCleanup(native.freelan_IPv4Endpoint_free, result)

        self.assertNotEqual(ffi.NULL, result)

    def test_IPv4Endpoint_from_string_incorrect_value(self):
        result = native.freelan_IPv4Endpoint_from_string(
            self.ectx,
            b"incorrect value",
        )

        self.assertEqual(ffi.NULL, result)

    def test_IPv4Endpoint_from_string_empty_value(self):
        result = native.freelan_IPv4Endpoint_from_string(self.ectx, b"")

        self.assertEqual(ffi.NULL, result)

    def test_IPv4Endpoint_from_parts(self):
        ip_address = native.freelan_IPv4Address_from_string(
            self.ectx,
            b"9.0.0.1",
        )
        self.addCleanup(native.freelan_IPv4Address_free, ip_address)
        port_number = native.freelan_PortNumber_from_string(
            self.ectx,
            b"12000",
        )
        self.addCleanup(native.freelan_PortNumber_free, port_number)
        result = native.freelan_IPv4Endpoint_from_parts(
            ip_address,
            port_number,
        )
        self.addCleanup(native.freelan_IPv4Endpoint_free, result)

        self.assertNotEqual(ffi.NULL, result)

    def test_IPv4Endpoint_to_string_simple(self):
        str_value = b"9.0.0.1:12000"

        value = native.freelan_IPv4Endpoint_from_string(self.ectx, str_value)
        self.addCleanup(native.freelan_IPv4Endpoint_free, value)

        result = native.freelan_IPv4Endpoint_to_string(self.ectx, value)
        self.addCleanup(native.freelan_free, result)

        self.assertEqual(str_value, ffi.string(result))

    def test_IPv4Endpoint_clone(self):
        inst = native.freelan_IPv4Endpoint_from_string(
            self.ectx,
            b"192.168.0.1:12000",
        )
        self.addCleanup(native.freelan_IPv4Endpoint_free, inst)

        result = native.freelan_IPv4Endpoint_clone(self.ectx, inst)
        self.addCleanup(native.freelan_IPv4Endpoint_free, result)

        self.assertNotEqual(ffi.NULL, result)

    def test_IPv4Endpoint_less_than(self):
        str_values = (b"9.0.0.1:12000", b"9.0.0.2:11000")

        values = (
            native.freelan_IPv4Endpoint_from_string(self.ectx, str_values[0]),
            native.freelan_IPv4Endpoint_from_string(self.ectx, str_values[1]),
        )
        self.addCleanup(native.freelan_IPv4Endpoint_free, values[0])
        self.addCleanup(native.freelan_IPv4Endpoint_free, values[1])

        result = native.freelan_IPv4Endpoint_less_than(*values)
        self.assertNotEqual(0, result)

    def test_IPv4Endpoint_equal(self):
        str_values = (b"9.0.0.1:12000", b"9.0.0.2:11000")

        values = (
            native.freelan_IPv4Endpoint_from_string(self.ectx, str_values[0]),
            native.freelan_IPv4Endpoint_from_string(self.ectx, str_values[1]),
        )
        self.addCleanup(native.freelan_IPv4Endpoint_free, values[0])
        self.addCleanup(native.freelan_IPv4Endpoint_free, values[1])

        result = native.freelan_IPv4Endpoint_equal(*values)
        self.assertEqual(0, result)

    def test_IPv4Endpoint_get_IPv4Address(self):
        endpoint = native.freelan_IPv4Endpoint_from_string(
            self.ectx,
            b"9.0.0.1:12000",
        )
        self.addCleanup(native.freelan_IPv4Endpoint_free, endpoint)
        result = native.freelan_IPv4Endpoint_get_IPv4Address(endpoint)
        self.addCleanup(native.freelan_IPv4Address_free, result)

        self.assertNotEqual(ffi.NULL, result)

    def test_IPv4Endpoint_get_PortNumber(self):
        endpoint = native.freelan_IPv4Endpoint_from_string(
            self.ectx,
            b"9.0.0.1:12000",
        )
        self.addCleanup(native.freelan_IPv4Endpoint_free, endpoint)
        result = native.freelan_IPv4Endpoint_get_PortNumber(endpoint)
        self.addCleanup(native.freelan_PortNumber_free, result)

        self.assertNotEqual(ffi.NULL, result)

    def test_IPv6Endpoint_from_string_simple(self):
        result = native.freelan_IPv6Endpoint_from_string(
            self.ectx,
            b"[fe80::a:1]:12000",
        )
        self.addCleanup(native.freelan_IPv6Endpoint_free, result)

        self.assertNotEqual(ffi.NULL, result)

    def test_IPv6Endpoint_from_string_incorrect_value(self):
        result = native.freelan_IPv6Endpoint_from_string(
            self.ectx,
            b"incorrect value",
        )

        self.assertEqual(ffi.NULL, result)

    def test_IPv6Endpoint_from_string_empty_value(self):
        result = native.freelan_IPv6Endpoint_from_string(self.ectx, b"")

        self.assertEqual(ffi.NULL, result)

    def test_IPv6Endpoint_from_parts(self):
        ip_address = native.freelan_IPv6Address_from_string(
            self.ectx,
            b"fe80::a:1",
        )
        self.addCleanup(native.freelan_IPv6Address_free, ip_address)
        port_number = native.freelan_PortNumber_from_string(
            self.ectx,
            b"12000",
        )
        self.addCleanup(native.freelan_PortNumber_free, port_number)
        result = native.freelan_IPv6Endpoint_from_parts(
            ip_address,
            port_number,
        )
        self.addCleanup(native.freelan_IPv6Endpoint_free, result)

        self.assertNotEqual(ffi.NULL, result)

    def test_IPv6Endpoint_to_string_simple(self):
        str_value = b"[fe80::a:1]:12000"

        value = native.freelan_IPv6Endpoint_from_string(self.ectx, str_value)
        self.addCleanup(native.freelan_IPv6Endpoint_free, value)

        result = native.freelan_IPv6Endpoint_to_string(self.ectx, value)
        self.addCleanup(native.freelan_free, result)

        self.assertEqual(str_value, ffi.string(result))

    def test_IPv6Endpoint_clone(self):
        inst = native.freelan_IPv6Endpoint_from_string(
            self.ectx,
            b"[fe80::a:1]:12000",
        )
        self.addCleanup(native.freelan_IPv6Endpoint_free, inst)

        result = native.freelan_IPv6Endpoint_clone(self.ectx, inst)
        self.addCleanup(native.freelan_IPv6Endpoint_free, result)

        self.assertNotEqual(ffi.NULL, result)

    def test_IPv6Endpoint_less_than(self):
        str_values = (b"[fe80::a:1]:12000", b"[fe80::a:2]:11000")

        values = (
            native.freelan_IPv6Endpoint_from_string(self.ectx, str_values[0]),
            native.freelan_IPv6Endpoint_from_string(self.ectx, str_values[1]),
        )
        self.addCleanup(native.freelan_IPv6Endpoint_free, values[0])
        self.addCleanup(native.freelan_IPv6Endpoint_free, values[1])

        result = native.freelan_IPv6Endpoint_less_than(*values)
        self.assertNotEqual(0, result)

    def test_IPv6Endpoint_equal(self):
        str_values = (b"[fe80::a:1]:12000", b"[fe80::a:2]:11000")

        values = (
            native.freelan_IPv6Endpoint_from_string(self.ectx, str_values[0]),
            native.freelan_IPv6Endpoint_from_string(self.ectx, str_values[1]),
        )
        self.addCleanup(native.freelan_IPv6Endpoint_free, values[0])
        self.addCleanup(native.freelan_IPv6Endpoint_free, values[1])

        result = native.freelan_IPv6Endpoint_equal(*values)
        self.assertEqual(0, result)

    def test_IPv6Endpoint_get_IPv6Address(self):
        endpoint = native.freelan_IPv6Endpoint_from_string(
            self.ectx,
            b"[fe80::a:1]:12000",
        )
        self.addCleanup(native.freelan_IPv6Endpoint_free, endpoint)
        result = native.freelan_IPv6Endpoint_get_IPv6Address(endpoint)
        self.addCleanup(native.freelan_IPv6Address_free, result)

        self.assertNotEqual(ffi.NULL, result)

    def test_IPv6Endpoint_get_PortNumber(self):
        endpoint = native.freelan_IPv6Endpoint_from_string(
            self.ectx,
            b"[fe80::a:1]:12000",
        )
        self.addCleanup(native.freelan_IPv6Endpoint_free, endpoint)
        result = native.freelan_IPv6Endpoint_get_PortNumber(endpoint)
        self.addCleanup(native.freelan_PortNumber_free, result)

        self.assertNotEqual(ffi.NULL, result)

    def test_HostnameEndpoint_from_string_simple(self):
        result = native.freelan_HostnameEndpoint_from_string(
            self.ectx,
            b"foo.bar:12000",
        )
        self.addCleanup(native.freelan_HostnameEndpoint_free, result)

        self.assertNotEqual(ffi.NULL, result)

    def test_HostnameEndpoint_from_string_incorrect_value(self):
        result = native.freelan_HostnameEndpoint_from_string(
            self.ectx,
            b"incorrect value",
        )

        self.assertEqual(ffi.NULL, result)

    def test_HostnameEndpoint_from_string_empty_value(self):
        result = native.freelan_HostnameEndpoint_from_string(self.ectx, b"")

        self.assertEqual(ffi.NULL, result)

    def test_HostnameEndpoint_from_parts(self):
        hostname = native.freelan_Hostname_from_string(self.ectx, b"foo.bar")
        self.addCleanup(native.freelan_Hostname_free, hostname)
        port_number = native.freelan_PortNumber_from_string(
            self.ectx,
            b"12000",
        )
        self.addCleanup(native.freelan_PortNumber_free, port_number)
        result = native.freelan_HostnameEndpoint_from_parts(
            hostname,
            port_number,
        )
        self.addCleanup(native.freelan_HostnameEndpoint_free, result)

        self.assertNotEqual(ffi.NULL, result)

    def test_HostnameEndpoint_to_string_simple(self):
        str_value = b"foo.bar:12000"

        value = native.freelan_HostnameEndpoint_from_string(
            self.ectx,
            str_value,
        )
        self.addCleanup(native.freelan_HostnameEndpoint_free, value)

        result = native.freelan_HostnameEndpoint_to_string(self.ectx, value)
        self.addCleanup(native.freelan_free, result)

        self.assertEqual(str_value, ffi.string(result))

    def test_HostnameEndpoint_clone(self):
        inst = native.freelan_HostnameEndpoint_from_string(
            self.ectx,
            b"some.hostname:12000",
        )
        self.addCleanup(native.freelan_HostnameEndpoint_free, inst)

        result = native.freelan_HostnameEndpoint_clone(self.ectx, inst)
        self.addCleanup(native.freelan_HostnameEndpoint_free, result)

        self.assertNotEqual(ffi.NULL, result)

    def test_HostnameEndpoint_less_than(self):
        str_values = (b"foo.bar:12000", b"foo.baz:11000")

        values = (
            native.freelan_HostnameEndpoint_from_string(
                self.ectx,
                str_values[0],
            ),
            native.freelan_HostnameEndpoint_from_string(
                self.ectx,
                str_values[1],
            ),
        )
        self.addCleanup(native.freelan_HostnameEndpoint_free, values[0])
        self.addCleanup(native.freelan_HostnameEndpoint_free, values[1])

        result = native.freelan_HostnameEndpoint_less_than(*values)
        self.assertNotEqual(0, result)

    def test_HostnameEndpoint_equal(self):
        str_values = (b"foo.bar:12000", b"foo.baz:11000")

        values = (
            native.freelan_HostnameEndpoint_from_string(
                self.ectx,
                str_values[0],
            ),
            native.freelan_HostnameEndpoint_from_string(
                self.ectx,
                str_values[1],
            ),
        )
        self.addCleanup(native.freelan_HostnameEndpoint_free, values[0])
        self.addCleanup(native.freelan_HostnameEndpoint_free, values[1])

        result = native.freelan_HostnameEndpoint_equal(*values)
        self.assertEqual(0, result)

    def test_HostnameEndpoint_get_Hostname(self):
        endpoint = native.freelan_HostnameEndpoint_from_string(
            self.ectx,
            b"foo.bar:12000",
        )
        self.addCleanup(native.freelan_HostnameEndpoint_free, endpoint)
        result = native.freelan_HostnameEndpoint_get_Hostname(endpoint)
        self.addCleanup(native.freelan_Hostname_free, result)

        self.assertNotEqual(ffi.NULL, result)

    def test_HostnameEndpoint_get_PortNumber(self):
        endpoint = native.freelan_HostnameEndpoint_from_string(
            self.ectx,
            b"foo.bar:12000",
        )
        self.addCleanup(native.freelan_HostnameEndpoint_free, endpoint)
        result = native.freelan_HostnameEndpoint_get_PortNumber(endpoint)
        self.addCleanup(native.freelan_PortNumber_free, result)

        self.assertNotEqual(ffi.NULL, result)

    def test_IPv4Route_from_string_simple(self):
        result = native.freelan_IPv4Route_from_string(self.ectx, b"9.0.0.1/24")
        self.addCleanup(native.freelan_IPv4Route_free, result)

        self.assertNotEqual(ffi.NULL, result)

    def test_IPv4Route_from_string_incorrect_value(self):
        result = native.freelan_IPv4Route_from_string(
            self.ectx,
            b"incorrect value",
        )

        self.assertEqual(ffi.NULL, result)

    def test_IPv4Route_from_string_empty_value(self):
        result = native.freelan_IPv4Route_from_string(self.ectx, b"")

        self.assertEqual(ffi.NULL, result)

    def test_IPv4Route_from_parts(self):
        ip_address = native.freelan_IPv4Address_from_string(
            self.ectx,
            b"9.0.0.1",
        )
        self.addCleanup(native.freelan_IPv4Address_free, ip_address)
        prefix_length = native.freelan_IPv4PrefixLength_from_string(
            self.ectx,
            b"24",
        )
        self.addCleanup(native.freelan_IPv4PrefixLength_free, prefix_length)
        result = native.freelan_IPv4Route_from_parts(
            ip_address,
            prefix_length,
            ffi.NULL,
        )
        self.addCleanup(native.freelan_IPv4Route_free, result)

        self.assertNotEqual(ffi.NULL, result)

    def test_IPv4Route_to_string_simple(self):
        str_value = b"9.0.0.0/24"

        value = native.freelan_IPv4Route_from_string(self.ectx, str_value)
        self.addCleanup(native.freelan_IPv4Route_free, value)

        result = native.freelan_IPv4Route_to_string(self.ectx, value)
        self.addCleanup(native.freelan_free, result)

        self.assertEqual(str_value, ffi.string(result))

    def test_IPv4Route_clone(self):
        inst = native.freelan_IPv4Route_from_string(self.ectx, b"10.0.0.0/24")
        self.addCleanup(native.freelan_IPv4Route_free, inst)

        result = native.freelan_IPv4Route_clone(self.ectx, inst)
        self.addCleanup(native.freelan_IPv4Route_free, result)

        self.assertNotEqual(ffi.NULL, result)

    def test_IPv4Route_less_than(self):
        str_values = (b"9.0.0.0/24", b"10.0.0.0/24")

        values = (
            native.freelan_IPv4Route_from_string(self.ectx, str_values[0]),
            native.freelan_IPv4Route_from_string(self.ectx, str_values[1]),
        )
        self.addCleanup(native.freelan_IPv4Route_free, values[0])
        self.addCleanup(native.freelan_IPv4Route_free, values[1])

        result = native.freelan_IPv4Route_less_than(*values)
        self.assertNotEqual(0, result)

    def test_IPv4Route_equal(self):
        str_values = (b"9.0.0.0/24", b"10.0.0.0/24")

        values = (
            native.freelan_IPv4Route_from_string(self.ectx, str_values[0]),
            native.freelan_IPv4Route_from_string(self.ectx, str_values[1]),
        )
        self.addCleanup(native.freelan_IPv4Route_free, values[0])
        self.addCleanup(native.freelan_IPv4Route_free, values[1])

        result = native.freelan_IPv4Route_equal(*values)
        self.assertEqual(0, result)

    def test_IPv4Route_get_IPv4Address(self):
        endpoint = native.freelan_IPv4Route_from_string(
            self.ectx,
            b"9.0.0.0/24",
        )
        self.addCleanup(native.freelan_IPv4Route_free, endpoint)
        result = native.freelan_IPv4Route_get_IPv4Address(endpoint)
        self.addCleanup(native.freelan_IPv4Address_free, result)

        self.assertNotEqual(ffi.NULL, result)

    def test_IPv4Route_get_IPv4PrefixLength(self):
        endpoint = native.freelan_IPv4Route_from_string(
            self.ectx,
            b"9.0.0.0/24",
        )
        self.addCleanup(native.freelan_IPv4Route_free, endpoint)
        result = native.freelan_IPv4Route_get_IPv4PrefixLength(endpoint)
        self.addCleanup(native.freelan_IPv4PrefixLength_free, result)

        self.assertNotEqual(ffi.NULL, result)

    def test_IPv4Route_get_IPv4Address_gateway(self):
        endpoint = native.freelan_IPv4Route_from_string(
            self.ectx,
            b"9.0.0.0/24@9.0.0.254",
        )
        self.addCleanup(native.freelan_IPv4Route_free, endpoint)
        result = native.freelan_IPv4Route_get_IPv4Address_gateway(endpoint)
        self.addCleanup(native.freelan_IPv4Address_free, result)

        self.assertNotEqual(ffi.NULL, result)

    def test_IPv6Route_from_string_simple(self):
        result = native.freelan_IPv6Route_from_string(
            self.ectx,
            b"fe80::a:0/120",
        )
        self.addCleanup(native.freelan_IPv6Route_free, result)

        self.assertNotEqual(ffi.NULL, result)

    def test_IPv6Route_from_string_incorrect_value(self):
        result = native.freelan_IPv6Route_from_string(
            self.ectx,
            b"incorrect value",
        )

        self.assertEqual(ffi.NULL, result)

    def test_IPv6Route_from_string_empty_value(self):
        result = native.freelan_IPv6Route_from_string(self.ectx, b"")

        self.assertEqual(ffi.NULL, result)

    def test_IPv6Route_from_parts(self):
        ip_address = native.freelan_IPv6Address_from_string(
            self.ectx,
            b"fe80::a:0",
        )
        self.addCleanup(native.freelan_IPv6Address_free, ip_address)
        prefix_length = native.freelan_IPv6PrefixLength_from_string(
            self.ectx,
            b"120",
        )
        self.addCleanup(native.freelan_IPv6PrefixLength_free, prefix_length)
        result = native.freelan_IPv6Route_from_parts(
            ip_address,
            prefix_length,
            ffi.NULL,
        )
        self.addCleanup(native.freelan_IPv6Route_free, result)

        self.assertNotEqual(ffi.NULL, result)

    def test_IPv6Route_to_string_simple(self):
        str_value = b"fe80::a:0/120"

        value = native.freelan_IPv6Route_from_string(self.ectx, str_value)
        self.addCleanup(native.freelan_IPv6Route_free, value)

        result = native.freelan_IPv6Route_to_string(self.ectx, value)
        self.addCleanup(native.freelan_free, result)

        self.assertEqual(str_value, ffi.string(result))

    def test_IPv6Route_clone(self):
        inst = native.freelan_IPv6Route_from_string(self.ectx, b"fe80::a:1/64")
        self.addCleanup(native.freelan_IPv6Route_free, inst)

        result = native.freelan_IPv6Route_clone(self.ectx, inst)
        self.addCleanup(native.freelan_IPv6Route_free, result)

        self.assertNotEqual(ffi.NULL, result)

    def test_IPv6Route_less_than(self):
        str_values = (b"fe80::a:0/120", b"fe90::b:0/120")

        values = (
            native.freelan_IPv6Route_from_string(self.ectx, str_values[0]),
            native.freelan_IPv6Route_from_string(self.ectx, str_values[1]),
        )
        self.addCleanup(native.freelan_IPv6Route_free, values[0])
        self.addCleanup(native.freelan_IPv6Route_free, values[1])

        result = native.freelan_IPv6Route_less_than(*values)
        self.assertNotEqual(0, result)

    def test_IPv6Route_equal(self):
        str_values = (b"fe80::a:0/120", b"fe90::a:0/120")

        values = (
            native.freelan_IPv6Route_from_string(self.ectx, str_values[0]),
            native.freelan_IPv6Route_from_string(self.ectx, str_values[1]),
        )
        self.addCleanup(native.freelan_IPv6Route_free, values[0])
        self.addCleanup(native.freelan_IPv6Route_free, values[1])

        result = native.freelan_IPv6Route_equal(*values)
        self.assertEqual(0, result)

    def test_IPv6Route_get_IPv6Address(self):
        endpoint = native.freelan_IPv6Route_from_string(
            self.ectx,
            b"fe80::a:0/120",
        )
        self.addCleanup(native.freelan_IPv6Route_free, endpoint)
        result = native.freelan_IPv6Route_get_IPv6Address(endpoint)
        self.addCleanup(native.freelan_IPv6Address_free, result)

        self.assertNotEqual(ffi.NULL, result)

    def test_IPv6Route_get_IPv6PrefixLength(self):
        endpoint = native.freelan_IPv6Route_from_string(
            self.ectx,
            b"fe80::a:0/120",
        )
        self.addCleanup(native.freelan_IPv6Route_free, endpoint)
        result = native.freelan_IPv6Route_get_IPv6PrefixLength(endpoint)
        self.addCleanup(native.freelan_IPv6PrefixLength_free, result)

        self.assertNotEqual(ffi.NULL, result)

    def test_IPv6Route_get_IPv6Address_gateway(self):
        endpoint = native.freelan_IPv6Route_from_string(
            self.ectx,
            b"fe80::a:0/120@fe80::a:ffff",
        )
        self.addCleanup(native.freelan_IPv6Route_free, endpoint)
        result = native.freelan_IPv6Route_get_IPv6Address_gateway(endpoint)
        self.addCleanup(native.freelan_IPv6Address_free, result)

        self.assertNotEqual(ffi.NULL, result)

    def test_IPAddress_from_string_simple(self):
        result = native.freelan_IPAddress_from_string(self.ectx, b"1.2.4.8")
        self.addCleanup(native.freelan_IPAddress_free, result)

        self.assertNotEqual(ffi.NULL, result)

    def test_IPAddress_from_IPv4Address(self):
        value = native.freelan_IPv4Address_from_string(self.ectx, b"1.2.4.8")
        self.addCleanup(native.freelan_IPv4Address_free, value)

        result = native.freelan_IPAddress_from_IPv4Address(value)
        self.addCleanup(native.freelan_IPAddress_free, result)

        self.assertNotEqual(ffi.NULL, result)

    def test_IPAddress_from_IPv6Address(self):
        value = native.freelan_IPv6Address_from_string(self.ectx, b"fe80::1:a")
        self.addCleanup(native.freelan_IPv6Address_free, value)

        result = native.freelan_IPAddress_from_IPv6Address(value)
        self.addCleanup(native.freelan_IPAddress_free, result)

        self.assertNotEqual(ffi.NULL, result)

    def test_IPAddress_as_IPv4Address(self):
        inst = native.freelan_IPAddress_from_string(self.ectx, b"1.2.4.8")
        self.addCleanup(native.freelan_IPAddress_free, inst)

        self.assertNotEqual(
            ffi.NULL,
            native.freelan_IPAddress_as_IPv4Address(inst),
        )
        self.assertEqual(
            ffi.NULL,
            native.freelan_IPAddress_as_IPv6Address(inst),
        )

    def test_IPAddress_as_IPv6Address(self):
        inst = native.freelan_IPAddress_from_string(self.ectx, b"fe80::a:1")
        self.addCleanup(native.freelan_IPAddress_free, inst)

        self.assertEqual(
            ffi.NULL,
            native.freelan_IPAddress_as_IPv4Address(inst),
        )
        self.assertNotEqual(
            ffi.NULL,
            native.freelan_IPAddress_as_IPv6Address(inst),
        )

    def test_IPAddress_from_string_truncated(self):
        result = native.freelan_IPAddress_from_string(self.ectx, b"127.1")

        self.assertEqual(ffi.NULL, result)

    def test_IPAddress_from_string_incorrect_value(self):
        result = native.freelan_IPAddress_from_string(
            self.ectx,
            b"incorrect value",
        )

        self.assertEqual(ffi.NULL, result)

    def test_IPAddress_from_string_empty_value(self):
        result = native.freelan_IPAddress_from_string(self.ectx, b"")

        self.assertEqual(ffi.NULL, result)

    def test_IPAddress_to_string_simple(self):
        str_value = b"1.2.4.8"

        value = native.freelan_IPAddress_from_string(self.ectx, str_value)
        self.addCleanup(native.freelan_IPAddress_free, value)

        result = native.freelan_IPAddress_to_string(self.ectx, value)
        self.addCleanup(native.freelan_free, result)

        self.assertEqual(str_value, ffi.string(result))

    def test_IPAddress_clone(self):
        inst = native.freelan_IPAddress_from_string(self.ectx, b"9.0.0.1")
        self.addCleanup(native.freelan_IPAddress_free, inst)

        result = native.freelan_IPAddress_clone(self.ectx, inst)
        self.addCleanup(native.freelan_IPAddress_free, result)

        self.assertNotEqual(ffi.NULL, result)

    def test_IPAddress_less_than(self):
        str_values = (b"1.2.4.8", b"1.2.4.9")

        values = (
            native.freelan_IPAddress_from_string(self.ectx, str_values[0]),
            native.freelan_IPAddress_from_string(self.ectx, str_values[1]),
        )
        self.addCleanup(native.freelan_IPAddress_free, values[0])
        self.addCleanup(native.freelan_IPAddress_free, values[1])

        result = native.freelan_IPAddress_less_than(*values)
        self.assertNotEqual(0, result)

    def test_IPAddress_equal(self):
        str_values = (b"1.2.4.8", b"1.2.4.9")

        values = (
            native.freelan_IPAddress_from_string(self.ectx, str_values[0]),
            native.freelan_IPAddress_from_string(self.ectx, str_values[1]),
        )
        self.addCleanup(native.freelan_IPAddress_free, values[0])
        self.addCleanup(native.freelan_IPAddress_free, values[1])

        result = native.freelan_IPAddress_equal(*values)
        self.assertEqual(0, result)

    def test_IPRoute_from_string_simple(self):
        result = native.freelan_IPRoute_from_string(self.ectx, b"9.0.0.0/24")
        self.addCleanup(native.freelan_IPRoute_free, result)

        self.assertNotEqual(ffi.NULL, result)

    def test_IPRoute_from_IPv4Route(self):
        value = native.freelan_IPv4Route_from_string(self.ectx, b"9.0.0.0/24")
        self.addCleanup(native.freelan_IPv4Route_free, value)

        result = native.freelan_IPRoute_from_IPv4Route(value)
        self.addCleanup(native.freelan_IPRoute_free, result)

        self.assertNotEqual(ffi.NULL, result)

    def test_IPRoute_from_IPv6Route(self):
        value = native.freelan_IPv6Route_from_string(
            self.ectx,
            b"fe80::1:a:0/120",
        )
        self.addCleanup(native.freelan_IPv6Route_free, value)

        result = native.freelan_IPRoute_from_IPv6Route(value)
        self.addCleanup(native.freelan_IPRoute_free, result)

        self.assertNotEqual(ffi.NULL, result)

    def test_IPRoute_as_IPv4Route(self):
        inst = native.freelan_IPRoute_from_string(self.ectx, b"9.0.0.0/24")
        self.addCleanup(native.freelan_IPRoute_free, inst)

        self.assertNotEqual(
            ffi.NULL,
            native.freelan_IPRoute_as_IPv4Route(inst),
        )
        self.assertEqual(
            ffi.NULL,
            native.freelan_IPRoute_as_IPv6Route(inst),
        )

    def test_IPRoute_as_IPv6Route(self):
        inst = native.freelan_IPRoute_from_string(self.ectx, b"fe80::a:0/120")
        self.addCleanup(native.freelan_IPRoute_free, inst)

        self.assertEqual(
            ffi.NULL,
            native.freelan_IPRoute_as_IPv4Route(inst),
        )
        self.assertNotEqual(
            ffi.NULL,
            native.freelan_IPRoute_as_IPv6Route(inst),
        )

    def test_IPRoute_from_string_incorrect_value(self):
        result = native.freelan_IPRoute_from_string(
            self.ectx,
            b"incorrect value",
        )

        self.assertEqual(ffi.NULL, result)

    def test_IPRoute_from_string_empty_value(self):
        result = native.freelan_IPRoute_from_string(self.ectx, b"")

        self.assertEqual(ffi.NULL, result)

    def test_IPRoute_to_string_simple(self):
        str_value = b"9.0.0.0/24"

        value = native.freelan_IPRoute_from_string(self.ectx, str_value)
        self.addCleanup(native.freelan_IPRoute_free, value)

        result = native.freelan_IPRoute_to_string(self.ectx, value)
        self.addCleanup(native.freelan_free, result)

        self.assertEqual(str_value, ffi.string(result))

    def test_IPRoute_clone(self):
        inst = native.freelan_IPRoute_from_string(self.ectx, b"9.0.0.0/24")
        self.addCleanup(native.freelan_IPRoute_free, inst)

        result = native.freelan_IPRoute_clone(self.ectx, inst)
        self.addCleanup(native.freelan_IPRoute_free, result)

        self.assertNotEqual(ffi.NULL, result)

    def test_IPRoute_less_than(self):
        str_values = (b"9.0.0.0/24", b"9.0.1.0/24")

        values = (
            native.freelan_IPRoute_from_string(self.ectx, str_values[0]),
            native.freelan_IPRoute_from_string(self.ectx, str_values[1]),
        )
        self.addCleanup(native.freelan_IPRoute_free, values[0])
        self.addCleanup(native.freelan_IPRoute_free, values[1])

        result = native.freelan_IPRoute_less_than(*values)
        self.assertNotEqual(0, result)

    def test_IPRoute_equal(self):
        str_values = (b"9.0.0.0/24", b"9.0.1.0/24")

        values = (
            native.freelan_IPRoute_from_string(self.ectx, str_values[0]),
            native.freelan_IPRoute_from_string(self.ectx, str_values[1]),
        )
        self.addCleanup(native.freelan_IPRoute_free, values[0])
        self.addCleanup(native.freelan_IPRoute_free, values[1])

        result = native.freelan_IPRoute_equal(*values)
        self.assertEqual(0, result)

    def test_Host_from_string_simple(self):
        result = native.freelan_Host_from_string(self.ectx, b"1.2.4.8")
        self.addCleanup(native.freelan_Host_free, result)

        self.assertNotEqual(ffi.NULL, result)

    def test_Host_from_IPv4Address(self):
        value = native.freelan_IPv4Address_from_string(self.ectx, b"1.2.4.8")
        self.addCleanup(native.freelan_IPv4Address_free, value)

        result = native.freelan_Host_from_IPv4Address(value)
        self.addCleanup(native.freelan_Host_free, result)

        self.assertNotEqual(ffi.NULL, result)

    def test_Host_from_IPv6Address(self):
        value = native.freelan_IPv6Address_from_string(self.ectx, b"fe80::1:a")
        self.addCleanup(native.freelan_IPv6Address_free, value)

        result = native.freelan_Host_from_IPv6Address(value)
        self.addCleanup(native.freelan_Host_free, result)

        self.assertNotEqual(ffi.NULL, result)

    def test_Host_from_Hostname(self):
        value = native.freelan_Hostname_from_string(self.ectx, b"foo.bar.net")
        self.addCleanup(native.freelan_Hostname_free, value)

        result = native.freelan_Host_from_Hostname(value)
        self.addCleanup(native.freelan_Host_free, result)

        self.assertNotEqual(ffi.NULL, result)

    def test_Host_as_IPv4Address(self):
        inst = native.freelan_Host_from_string(self.ectx, b"1.2.4.8")
        self.addCleanup(native.freelan_Host_free, inst)

        self.assertNotEqual(
            ffi.NULL,
            native.freelan_Host_as_IPv4Address(inst),
        )
        self.assertEqual(
            ffi.NULL,
            native.freelan_Host_as_IPv6Address(inst),
        )
        self.assertEqual(
            ffi.NULL,
            native.freelan_Host_as_Hostname(inst),
        )

    def test_Host_as_IPv6Address(self):
        inst = native.freelan_Host_from_string(self.ectx, b"fe80::a:1")
        self.addCleanup(native.freelan_Host_free, inst)

        self.assertEqual(
            ffi.NULL,
            native.freelan_Host_as_IPv4Address(inst),
        )
        self.assertNotEqual(
            ffi.NULL,
            native.freelan_Host_as_IPv6Address(inst),
        )
        self.assertEqual(
            ffi.NULL,
            native.freelan_Host_as_Hostname(inst),
        )

    def test_Host_as_Hostname(self):
        inst = native.freelan_Host_from_string(self.ectx, b"foo.bar.net")
        self.addCleanup(native.freelan_Host_free, inst)

        self.assertEqual(
            ffi.NULL,
            native.freelan_Host_as_IPv4Address(inst),
        )
        self.assertEqual(
            ffi.NULL,
            native.freelan_Host_as_IPv6Address(inst),
        )
        self.assertNotEqual(
            ffi.NULL,
            native.freelan_Host_as_Hostname(inst),
        )

    def test_Host_from_string_truncated(self):
        result = native.freelan_Host_from_string(self.ectx, b"127.1")

        self.assertEqual(ffi.NULL, result)

    def test_Host_from_string_incorrect_value(self):
        result = native.freelan_Host_from_string(self.ectx, b"incorrect value")

        self.assertEqual(ffi.NULL, result)

    def test_Host_from_string_empty_value(self):
        result = native.freelan_Host_from_string(self.ectx, b"")

        self.assertEqual(ffi.NULL, result)

    def test_Host_to_string_simple(self):
        str_value = b"1.2.4.8"

        value = native.freelan_Host_from_string(self.ectx, str_value)
        self.addCleanup(native.freelan_Host_free, value)

        result = native.freelan_Host_to_string(self.ectx, value)
        self.addCleanup(native.freelan_free, result)

        self.assertEqual(str_value, ffi.string(result))

    def test_Host_clone(self):
        inst = native.freelan_Host_from_string(self.ectx, b"9.0.0.1")
        self.addCleanup(native.freelan_Host_free, inst)

        result = native.freelan_Host_clone(self.ectx, inst)
        self.addCleanup(native.freelan_Host_free, result)

        self.assertNotEqual(ffi.NULL, result)

    def test_Host_less_than(self):
        str_values = (b"1.2.4.8", b"1.2.4.9")

        values = (
            native.freelan_Host_from_string(self.ectx, str_values[0]),
            native.freelan_Host_from_string(self.ectx, str_values[1]),
        )
        self.addCleanup(native.freelan_Host_free, values[0])
        self.addCleanup(native.freelan_Host_free, values[1])

        result = native.freelan_Host_less_than(*values)
        self.assertNotEqual(0, result)

    def test_Host_equal(self):
        str_values = (b"1.2.4.8", b"1.2.4.9")

        values = (
            native.freelan_Host_from_string(self.ectx, str_values[0]),
            native.freelan_Host_from_string(self.ectx, str_values[1]),
        )
        self.addCleanup(native.freelan_Host_free, values[0])
        self.addCleanup(native.freelan_Host_free, values[1])

        result = native.freelan_Host_equal(*values)
        self.assertEqual(0, result)

    def test_IPEndpoint_from_string_simple(self):
        result = native.freelan_IPEndpoint_from_string(
            self.ectx,
            b"1.2.4.8:12000",
        )
        self.addCleanup(native.freelan_IPEndpoint_free, result)

        self.assertNotEqual(ffi.NULL, result)

    def test_IPEndpoint_from_IPv4Endpoint(self):
        value = native.freelan_IPv4Endpoint_from_string(
            self.ectx,
            b"1.2.4.8:12000",
        )
        self.addCleanup(native.freelan_IPv4Endpoint_free, value)

        result = native.freelan_IPEndpoint_from_IPv4Endpoint(value)
        self.addCleanup(native.freelan_IPEndpoint_free, result)

        self.assertNotEqual(ffi.NULL, result)

    def test_IPEndpoint_from_IPv6Endpoint(self):
        value = native.freelan_IPv6Endpoint_from_string(
            self.ectx,
            b"[fe80::1:a]:12000",
        )
        self.addCleanup(native.freelan_IPv6Endpoint_free, value)

        result = native.freelan_IPEndpoint_from_IPv6Endpoint(value)
        self.addCleanup(native.freelan_IPEndpoint_free, result)

        self.assertNotEqual(ffi.NULL, result)

    def test_IPEndpoint_as_IPv4Endpoint(self):
        inst = native.freelan_IPEndpoint_from_string(
            self.ectx,
            b"1.2.4.8:12000",
        )
        self.addCleanup(native.freelan_IPEndpoint_free, inst)

        self.assertNotEqual(
            ffi.NULL,
            native.freelan_IPEndpoint_as_IPv4Endpoint(inst),
        )
        self.assertEqual(
            ffi.NULL,
            native.freelan_IPEndpoint_as_IPv6Endpoint(inst),
        )

    def test_IPEndpoint_as_IPv6Endpoint(self):
        inst = native.freelan_IPEndpoint_from_string(
            self.ectx,
            b"[fe80::a:1]:12000",
        )
        self.addCleanup(native.freelan_IPEndpoint_free, inst)

        self.assertEqual(
            ffi.NULL,
            native.freelan_IPEndpoint_as_IPv4Endpoint(inst),
        )
        self.assertNotEqual(
            ffi.NULL,
            native.freelan_IPEndpoint_as_IPv6Endpoint(inst),
        )

    def test_IPEndpoint_from_string_incorrect_value(self):
        result = native.freelan_IPEndpoint_from_string(
            self.ectx,
            b"incorrect value",
        )

        self.assertEqual(ffi.NULL, result)

    def test_IPEndpoint_from_string_empty_value(self):
        result = native.freelan_IPEndpoint_from_string(self.ectx, b"")

        self.assertEqual(ffi.NULL, result)

    def test_IPEndpoint_to_string_simple(self):
        str_value = b"1.2.4.8:12000"

        value = native.freelan_IPEndpoint_from_string(self.ectx, str_value)
        self.addCleanup(native.freelan_IPEndpoint_free, value)

        result = native.freelan_IPEndpoint_to_string(self.ectx, value)
        self.addCleanup(native.freelan_free, result)

        self.assertEqual(str_value, ffi.string(result))

    def test_IPEndpoint_clone(self):
        inst = native.freelan_IPEndpoint_from_string(
            self.ectx,
            b"9.0.0.1:12000",
        )
        self.addCleanup(native.freelan_IPEndpoint_free, inst)

        result = native.freelan_IPEndpoint_clone(self.ectx, inst)
        self.addCleanup(native.freelan_IPEndpoint_free, result)

        self.assertNotEqual(ffi.NULL, result)

    def test_IPEndpoint_less_than(self):
        str_values = (b"1.2.4.8:12000", b"1.2.4.9:12000")

        values = (
            native.freelan_IPEndpoint_from_string(self.ectx, str_values[0]),
            native.freelan_IPEndpoint_from_string(self.ectx, str_values[1]),
        )
        self.addCleanup(native.freelan_IPEndpoint_free, values[0])
        self.addCleanup(native.freelan_IPEndpoint_free, values[1])

        result = native.freelan_IPEndpoint_less_than(*values)
        self.assertNotEqual(0, result)

    def test_IPEndpoint_equal(self):
        str_values = (b"1.2.4.8:12000", b"1.2.4.9:12000")

        values = (
            native.freelan_IPEndpoint_from_string(self.ectx, str_values[0]),
            native.freelan_IPEndpoint_from_string(self.ectx, str_values[1]),
        )
        self.addCleanup(native.freelan_IPEndpoint_free, values[0])
        self.addCleanup(native.freelan_IPEndpoint_free, values[1])

        result = native.freelan_IPEndpoint_equal(*values)
        self.assertEqual(0, result)

    def test_HostEndpoint_from_string_simple(self):
        result = native.freelan_HostEndpoint_from_string(
            self.ectx,
            b"1.2.4.8:12000",
        )
        self.addCleanup(native.freelan_HostEndpoint_free, result)

        self.assertNotEqual(ffi.NULL, result)

    def test_HostEndpoint_from_IPv4Endpoint(self):
        value = native.freelan_IPv4Endpoint_from_string(
            self.ectx,
            b"1.2.4.8:12000",
        )
        self.addCleanup(native.freelan_IPv4Endpoint_free, value)

        result = native.freelan_HostEndpoint_from_IPv4Endpoint(value)
        self.addCleanup(native.freelan_HostEndpoint_free, result)

        self.assertNotEqual(ffi.NULL, result)

    def test_HostEndpoint_from_IPv6Endpoint(self):
        value = native.freelan_IPv6Endpoint_from_string(
            self.ectx,
            b"[fe80::1:a]:12000",
        )
        self.addCleanup(native.freelan_IPv6Endpoint_free, value)

        result = native.freelan_HostEndpoint_from_IPv6Endpoint(value)
        self.addCleanup(native.freelan_HostEndpoint_free, result)

        self.assertNotEqual(ffi.NULL, result)

    def test_HostEndpoint_from_HostnameEndpoint(self):
        value = native.freelan_HostnameEndpoint_from_string(
            self.ectx,
            b"foo.bar.net:12000",
        )
        self.addCleanup(native.freelan_HostnameEndpoint_free, value)

        result = native.freelan_HostEndpoint_from_HostnameEndpoint(value)
        self.addCleanup(native.freelan_HostEndpoint_free, result)

        self.assertNotEqual(ffi.NULL, result)

    def test_HostEndpoint_as_IPv4Endpoint(self):
        inst = native.freelan_HostEndpoint_from_string(
            self.ectx,
            b"1.2.4.8:12000",
        )
        self.addCleanup(native.freelan_HostEndpoint_free, inst)

        self.assertNotEqual(
            ffi.NULL,
            native.freelan_HostEndpoint_as_IPv4Endpoint(inst),
        )
        self.assertEqual(
            ffi.NULL,
            native.freelan_HostEndpoint_as_IPv6Endpoint(inst),
        )
        self.assertEqual(
            ffi.NULL,
            native.freelan_HostEndpoint_as_HostnameEndpoint(inst),
        )

    def test_HostEndpoint_as_IPv6Endpoint(self):
        inst = native.freelan_HostEndpoint_from_string(
            self.ectx,
            b"[fe80::a:1]:12000",
        )
        self.addCleanup(native.freelan_HostEndpoint_free, inst)

        self.assertEqual(
            ffi.NULL,
            native.freelan_HostEndpoint_as_IPv4Endpoint(inst),
        )
        self.assertNotEqual(
            ffi.NULL,
            native.freelan_HostEndpoint_as_IPv6Endpoint(inst),
        )
        self.assertEqual(
            ffi.NULL,
            native.freelan_HostEndpoint_as_HostnameEndpoint(inst),
        )

    def test_HostEndpoint_as_HostnameEndpoint(self):
        inst = native.freelan_HostEndpoint_from_string(
            self.ectx,
            b"foo.bar.net:12000",
        )
        self.addCleanup(native.freelan_HostEndpoint_free, inst)

        self.assertEqual(
            ffi.NULL,
            native.freelan_HostEndpoint_as_IPv4Endpoint(inst),
        )
        self.assertEqual(
            ffi.NULL,
            native.freelan_HostEndpoint_as_IPv6Endpoint(inst),
        )
        self.assertNotEqual(
            ffi.NULL,
            native.freelan_HostEndpoint_as_HostnameEndpoint(inst),
        )

    def test_HostEndpoint_from_string_incorrect_value(self):
        result = native.freelan_HostEndpoint_from_string(
            self.ectx,
            b"incorrect value",
        )

        self.assertEqual(ffi.NULL, result)

    def test_HostEndpoint_from_string_empty_value(self):
        result = native.freelan_HostEndpoint_from_string(self.ectx, b"")

        self.assertEqual(ffi.NULL, result)

    def test_HostEndpoint_to_string_simple(self):
        str_value = b"1.2.4.8:12000"

        value = native.freelan_HostEndpoint_from_string(self.ectx, str_value)
        self.addCleanup(native.freelan_HostEndpoint_free, value)

        result = native.freelan_HostEndpoint_to_string(self.ectx, value)
        self.addCleanup(native.freelan_free, result)

        self.assertEqual(str_value, ffi.string(result))

    def test_HostEndpoint_clone(self):
        inst = native.freelan_HostEndpoint_from_string(
            self.ectx,
            b"9.0.0.1:12000",
        )
        self.addCleanup(native.freelan_HostEndpoint_free, inst)

        result = native.freelan_HostEndpoint_clone(self.ectx, inst)
        self.addCleanup(native.freelan_HostEndpoint_free, result)

        self.assertNotEqual(ffi.NULL, result)

    def test_HostEndpoint_less_than(self):
        str_values = (b"1.2.4.8:12000", b"1.2.4.9:12000")

        values = (
            native.freelan_HostEndpoint_from_string(self.ectx, str_values[0]),
            native.freelan_HostEndpoint_from_string(self.ectx, str_values[1]),
        )
        self.addCleanup(native.freelan_HostEndpoint_free, values[0])
        self.addCleanup(native.freelan_HostEndpoint_free, values[1])

        result = native.freelan_HostEndpoint_less_than(*values)
        self.assertNotEqual(0, result)

    def test_HostEndpoint_equal(self):
        str_values = (b"1.2.4.8:12000", b"1.2.4.9:12000")

        values = (
            native.freelan_HostEndpoint_from_string(self.ectx, str_values[0]),
            native.freelan_HostEndpoint_from_string(self.ectx, str_values[1]),
        )
        self.addCleanup(native.freelan_HostEndpoint_free, values[0])
        self.addCleanup(native.freelan_HostEndpoint_free, values[1])

        result = native.freelan_HostEndpoint_equal(*values)
        self.assertEqual(0, result)
