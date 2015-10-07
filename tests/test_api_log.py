# coding=utf-8

"""
Test the API log functions and helpers.
"""

from __future__ import unicode_literals

from mock import ANY
from unittest import TestCase
from datetime import datetime
from mock import (
    patch,
    MagicMock,
    call,
)

from pyfreelan.api import (
    ffi,
    native,
)
from pyfreelan.api.log import (
    LogLevel,
    utc_datetime_to_utc_timestamp,
    utc_timestamp_to_utc_datetime,
    to_c,
    from_c,
    log_attach,
    log,
    set_log_function,
    from_native_payload,
    log_function,
    c_log_function,
)


class UTCDatetimeToUTCTimestampTests(TestCase):

    def test_epoch(self):
        epoch = datetime(1970, 1, 1)
        result = utc_datetime_to_utc_timestamp(epoch)
        self.assertAlmostEqual(0.0, result)

    def test_known_conversion(self):
        date = datetime(2015, 9, 12, 16, 0, 0)
        result = utc_datetime_to_utc_timestamp(date)
        self.assertAlmostEqual(1442073600.0, result)


class UTCTimestampToUTCDatetimeTests(TestCase):

    def test_epoch(self):
        epoch = datetime(1970, 1, 1)
        result = utc_timestamp_to_utc_datetime(0.0)
        self.assertEqual(epoch, result)

    def test_known_conversion(self):
        date = datetime(2015, 9, 12, 16, 0, 0)
        result = utc_timestamp_to_utc_datetime(1442073600.0)
        self.assertEqual(date, result)


class ToCTests(TestCase):

    def test_string(self):
        value = "foo".encode('utf-8')
        result = to_c(value)
        self.assertEqual(value, result)

    def test_unicode(self):
        value = "éléphant"
        result = to_c(value)
        self.assertEqual(value.encode('utf-8'), result)

    def test_integer(self):
        value = 42
        result = to_c(value)
        self.assertEqual(value, result)


class FromCTests(TestCase):

    def test_string(self):
        value = "foo"
        result = from_c(value.encode('utf-8'))
        self.assertEqual(value, result)

    def test_unicode(self):
        value = "éléphant"
        result = from_c(value)
        self.assertEqual(value, result)

    def test_integer(self):
        value = 42
        result = from_c(value)
        self.assertEqual(value, result)


class LogAttachTests(TestCase):

    def test_attach_unicode_key(self):
        some_french = "éléphant"
        some_encoded_french = some_french.encode('utf-8')
        entry = MagicMock()
        registry = []

        with patch("pyfreelan.api.log.native") as native_mock:
            with patch("pyfreelan.api.log.ffi") as ffi_mock:
                log_attach(
                    registry=registry,
                    entry=entry,
                    key=some_french,
                    value="foo",
                )

        ffi_mock.new.assert_called_once_with("const char[]", b"foo")
        self.assertEqual([ffi_mock.new("const char[]", b"foo")], registry)
        native_mock.freelan_log_attach.assert_called_once_with(
            entry,
            some_encoded_french,
            native_mock.FREELAN_LOG_PAYLOAD_TYPE_STRING,
            {'as_string': ffi_mock.new("const char[]", "foo")},
        )

    def test_attach_non_string_key(self):
        entry = MagicMock()

        with self.assertRaises(TypeError):
            log_attach(registry=[], entry=entry, key=42, value="foo")

    def test_attach_unicode_value(self):
        some_french = u"éléphant"
        some_encoded_french = some_french.encode('utf-8')
        entry = MagicMock()
        registry = []

        with patch("pyfreelan.api.log.native") as native_mock:
            with patch("pyfreelan.api.log.ffi") as ffi_mock:
                log_attach(
                    registry=registry,
                    entry=entry,
                    key="foo",
                    value=some_french,
                )

        ffi_mock.new.assert_called_once_with(
            "const char[]",
            some_encoded_french,
        )
        self.assertEqual(
            [ffi_mock.new("const char[]", some_encoded_french)],
            registry,
        )
        native_mock.freelan_log_attach.assert_called_once_with(
            entry,
            b"foo",
            native_mock.FREELAN_LOG_PAYLOAD_TYPE_STRING,
            {'as_string': ffi_mock.new("const char[]", some_encoded_french)},
        )

    def test_attach_string_value(self):
        entry = MagicMock()
        registry = []

        with patch("pyfreelan.api.log.native") as native_mock:
            with patch("pyfreelan.api.log.ffi") as ffi_mock:
                log_attach(
                    registry=registry,
                    entry=entry,
                    key="foo",
                    value="bar",
                )

        ffi_mock.new.assert_called_once_with("const char[]", b"bar")
        self.assertEqual([ffi_mock.new("const char[]", "bar")], registry)
        native_mock.freelan_log_attach.assert_called_once_with(
            entry,
            b"foo",
            native_mock.FREELAN_LOG_PAYLOAD_TYPE_STRING,
            {'as_string': ffi_mock.new("const char[]", "bar")},
        )

    def test_attach_boolean_value(self):
        entry = MagicMock()
        registry = []

        with patch("pyfreelan.api.log.native") as native_mock:
            log_attach(registry=registry, entry=entry, key="foo", value=True)

        self.assertEqual([], registry)
        native_mock.freelan_log_attach.assert_called_once_with(
            entry,
            b"foo",
            native_mock.FREELAN_LOG_PAYLOAD_TYPE_BOOLEAN,
            {'as_boolean': True},
        )

    def test_attach_integer_value(self):
        entry = MagicMock()
        registry = []

        with patch("pyfreelan.api.log.native") as native_mock:
            log_attach(registry=registry, entry=entry, key="foo", value=42)

        self.assertEqual([], registry)
        native_mock.freelan_log_attach.assert_called_once_with(
            entry,
            b"foo",
            native_mock.FREELAN_LOG_PAYLOAD_TYPE_INTEGER,
            {'as_integer': 42},
        )

    def test_attach_float_value(self):
        entry = MagicMock()
        registry = []

        with patch("pyfreelan.api.log.native") as native_mock:
            log_attach(registry=registry, entry=entry, key="foo", value=3.14)

        self.assertEqual([], registry)
        native_mock.freelan_log_attach.assert_called_once_with(
            entry,
            b"foo",
            native_mock.FREELAN_LOG_PAYLOAD_TYPE_FLOAT,
            {'as_float': 3.14},
        )

    def test_attach_none_value(self):
        entry = MagicMock()
        registry = []

        with patch("pyfreelan.api.log.native") as native_mock:
            log_attach(registry=registry, entry=entry, key="foo", value=None)

        self.assertEqual([], registry)
        native_mock.freelan_log_attach.assert_called_once_with(
            entry,
            b"foo",
            native_mock.FREELAN_LOG_PAYLOAD_TYPE_NULL,
            {'as_null': ffi.NULL},
        )

    def test_attach_invalid_value(self):
        entry = MagicMock()

        with self.assertRaises(TypeError):
            log_attach(registry=[], entry=entry, key="foo", value=MagicMock())


class LogTests(TestCase):

    def test_log_no_file_no_payload(self):
        with patch("pyfreelan.api.log.native") as native_mock:
            native_mock.freelan_log.return_value = True
            result = log(
                LogLevel.information,
                "generic",
                "mycode",
                timestamp=datetime(1970, 1, 1),
            )

        native_mock.freelan_log.called_once_with(
            LogLevel.information,
            0.0,
            "generic",
            "mycode",
            0,
            ffi.NULL,
            ffi.NULL,
            0,
        )
        self.assertTrue(result)

    def test_log_file_no_payload(self):
        with patch("pyfreelan.api.log.native") as native_mock:
            native_mock.freelan_log.return_value = False
            result = log(
                LogLevel.information,
                "generic",
                "mycode",
                timestamp=datetime(1970, 1, 1),
                file="myfile",
                line=123,
            )

        native_mock.freelan_log.called_once_with(
            LogLevel.information,
            0.0,
            "generic",
            "mycode",
            0,
            ffi.NULL,
            "myfile",
            123,
        )
        self.assertFalse(result)

    def test_log_payload(self):
        patcher = patch("pyfreelan.api.log.native")
        native_mock = patcher.start()
        self.addCleanup(patcher.stop)

        native_mock.freelan_log_complete.return_value = True

        patcher = patch("pyfreelan.api.log.log_attach")
        log_attach_mock = patcher.start()
        self.addCleanup(patcher.stop)

        payload = {
            'a': "value",
            'b': "éléphant",
            'c': 42,
            'd': 3.14,
            'e': True,
        }
        result = log(
            LogLevel.information,
            "generic",
            "mycode",
            payload=payload,
            timestamp=datetime(1970, 1, 1),
        )

        native_mock.freelan_log_start.assert_called_once_with(
            LogLevel.information.value,
            0.0,
            b"generic",
            b"mycode",
            ffi.NULL,
            0,
        )
        entry = native_mock.freelan_log_start.return_value
        self.assertEqual(
            [
                call([], entry, key, value)
                for key, value in payload.items()
            ],
            log_attach_mock.mock_calls,
        )
        native_mock.freelan_log_complete.assert_called_once_with(entry)
        self.assertTrue(result)

    def test_log_payload_with_exception(self):
        patcher = patch("pyfreelan.api.log.native")
        native_mock = patcher.start()
        self.addCleanup(patcher.stop)

        native_mock.freelan_log_complete.return_value = True

        real_log_attach = log_attach

        def my_log_attach(registry, entry, key, value):
            if key == 'c':
                raise RuntimeError
            else:
                return real_log_attach(registry, entry, key, value)

        patcher = patch(
            "pyfreelan.api.log.log_attach",
            side_effect=my_log_attach,
        )
        log_attach_mock = patcher.start()
        self.addCleanup(patcher.stop)

        payload = {
            'a': "value",
            'b': "éléphant",
            'c': 42,
            'd': 3.14,
            'e': True,
        }
        result = log(
            LogLevel.information,
            "generic",
            "mycode",
            payload=payload,
            timestamp=datetime(1970, 1, 1),
        )

        native_mock.freelan_log_start.assert_called_once_with(
            LogLevel.information.value,
            0.0,
            b"generic",
            b"mycode",
            ffi.NULL,
            0,
        )
        entry = native_mock.freelan_log_start.return_value
        self.assertEqual(
            [
                call(ANY, entry, key, value)
                for key, value in payload.items()
            ],
            log_attach_mock.mock_calls[:],
        )
        native_mock.freelan_log_complete.assert_called_once_with(entry)
        self.assertTrue(result)


class LoggingCallbackTests(TestCase):

    def setUp(self):
        null = ffi.NULL
        patcher = patch("pyfreelan.api.log.ffi")
        ffi_mock = patcher.start()
        self.addCleanup(patcher.stop)

        ffi_mock.NULL = null
        ffi_mock.string = lambda x: x

    def test_set_log_function(self):
        func = MagicMock()
        callbacks = {}

        with patch(
            "pyfreelan.api.log.CALLBACKS",
            callbacks,
        ):
            with patch("pyfreelan.api.log.native") as native_mock:
                set_log_function(func)

        native_mock.freelan_set_log_function.assert_called_once_with(
            c_log_function,
        )
        self.assertEqual(func, callbacks['log_function'])

    def test_reset_log_function(self):
        callbacks = {}

        with patch(
            "pyfreelan.api.log.CALLBACKS",
            callbacks,
        ):
            with patch("pyfreelan.api.log.native") as native_mock:
                set_log_function(None)

        native_mock.freelan_set_log_function.assert_called_once_with(ffi.NULL)
        self.assertEqual(None, callbacks['log_function'])

    def test_log_function_no_log_function(self):
        payload = MagicMock()

        with patch(
            "pyfreelan.api.log.from_native_payload",
        ) as from_native_payload_func:
            result = log_function(
                native.FREELAN_LOG_LEVEL_ERROR,
                0.0,
                "mydomain",
                "mycode",
                3,
                payload,
                "myfile",
                123,
            )

        self.assertEqual(0, result)
        self.assertEqual([], from_native_payload_func.mock_calls)

    def test_log_function_with_log_function(self):
        payload = [
            ("a", "one"),
            ("b", 2),
            ("c", 3.0),
            ("d", False),
        ]

        def local_from_native_payload(payload):
            return payload

        log_f = MagicMock()

        with patch(
            "pyfreelan.api.log.CALLBACKS",
            {'log_function': log_f},
        ):
            with patch(
                "pyfreelan.api.log.from_native_payload",
                side_effect=local_from_native_payload,
            ) as from_native_payload_func:
                result = log_function(
                    native.FREELAN_LOG_LEVEL_ERROR,
                    0.0,
                    "mydomain",
                    "mycode",
                    len(payload),
                    payload,
                    "myfile",
                    123,
                )

        self.assertEqual(1, result)
        self.assertEqual(
            [call(p) for p in payload],
            from_native_payload_func.mock_calls,
        )
        log_f.assert_called_once_with(
            domain='mydomain',
            code='mycode',
            level=LogLevel.error,
            timestamp=datetime(1970, 1, 1, 0, 0),
            file='myfile',
            line=123,
            payload=dict(payload),
        )

    def test_from_native_payload_string(self):
        payload = MagicMock()
        payload.key = b"foo"
        payload.type = native.FREELAN_LOG_PAYLOAD_TYPE_STRING
        payload.value = MagicMock()
        setattr(payload.value, 'as_string', b"bar")

        key, value = from_native_payload(payload)

        self.assertEqual("foo", key)
        self.assertEqual(u"bar", value)

    def test_from_native_payload_unicode_string(self):
        payload = MagicMock()
        payload.key = b"foo"
        payload.type = native.FREELAN_LOG_PAYLOAD_TYPE_STRING
        payload.value = MagicMock()
        setattr(payload.value, 'as_string', "éléphant".encode('utf-8'))

        key, value = from_native_payload(payload)

        self.assertEqual("foo", key)
        self.assertEqual("éléphant", value)

    def test_from_native_payload_integer(self):
        payload = MagicMock()
        payload.key = b"foo"
        payload.type = native.FREELAN_LOG_PAYLOAD_TYPE_INTEGER
        payload.value = MagicMock()
        setattr(payload.value, 'as_integer', 42)

        key, value = from_native_payload(payload)

        self.assertEqual("foo", key)
        self.assertEqual(42, value)

    def test_from_native_payload_float(self):
        payload = MagicMock()
        payload.key = b"foo"
        payload.type = native.FREELAN_LOG_PAYLOAD_TYPE_FLOAT
        payload.value = MagicMock()
        setattr(payload.value, 'as_float', 3.14)

        key, value = from_native_payload(payload)

        self.assertEqual("foo", key)
        self.assertEqual(3.14, value)

    def test_from_native_payload_boolean(self):
        payload = MagicMock()
        payload.key = b"foo"
        payload.type = native.FREELAN_LOG_PAYLOAD_TYPE_BOOLEAN
        payload.value = MagicMock()
        setattr(payload.value, 'as_boolean', 1)

        key, value = from_native_payload(payload)

        self.assertEqual("foo", key)
        self.assertEqual(True, value)

    def test_from_native_payload_null(self):
        payload = MagicMock()
        payload.key = b"foo"
        payload.type = native.FREELAN_LOG_PAYLOAD_TYPE_NULL
        payload.value = {'as_null': ffi.NULL}

        key, value = from_native_payload(payload)

        self.assertEqual("foo", key)
        self.assertIsNone(value)


class IntegrationTests(TestCase):

    def test_full_logging_sequence(self):
        func = MagicMock()

        set_log_function(func)
        self.addCleanup(set_log_function, None)

        log(
            LogLevel.error,
            u"télé",
            u"éléphant",
            payload={
                'a': 'alpha',
                'b': u"nénuphar",
                'c': 42,
                'd': 3.14,
                'e': False,
                'f': None,
            },
            timestamp=datetime(1970, 1, 2),
        )
        func.assert_called_once_with(
            level=LogLevel.error,
            domain=u"télé",
            code=u"éléphant",
            payload={
                'a': 'alpha',
                'b': u"nénuphar",
                'c': 42,
                'd': 3.14,
                'e': False,
                'f': None,
            },
            timestamp=datetime(1970, 1, 2),
            file=None,
            line=0,
        )
