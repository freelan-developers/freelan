# coding=utf-8
"""
Test the API log functions and helpers.
"""

from unittest import TestCase
from datetime import datetime
from mock import (
    patch,
    MagicMock,
    call,
)

from pyfreelan.api import (
    ffi,
)
from pyfreelan.api.log import (
    LogLevel,
    LogDomain,
    utc_datetime_to_utc_timestamp,
    utc_timestamp_to_utc_datetime,
    log_attach,
    log,
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


class LogAttachTests(TestCase):
    def test_attach_unicode_key(self):
        some_french = u"éléphant"
        some_encoded_french = some_french.encode('utf-8')
        entry = MagicMock()

        with patch(
            "pyfreelan.api.log.native.freelan_log_attach_string",
        ) as log_attach_string:
            log_attach(entry=entry, key=some_french, value="foo")

        log_attach_string.assert_called_once_with(
            entry,
            some_encoded_french,
            "foo",
        )

    def test_attach_non_string_key(self):
        entry = MagicMock()

        with self.assertRaises(TypeError):
            log_attach(entry=entry, key=42, value="foo")

    def test_attach_unicode_value(self):
        some_french = u"éléphant"
        some_encoded_french = some_french.encode('utf-8')
        entry = MagicMock()

        with patch(
            "pyfreelan.api.log.native.freelan_log_attach_string",
        ) as log_attach_string:
            log_attach(entry=entry, key="foo", value=some_french)

        log_attach_string.assert_called_once_with(
            entry,
            "foo",
            some_encoded_french,
        )

    def test_attach_string_value(self):
        entry = MagicMock()

        with patch(
            "pyfreelan.api.log.native.freelan_log_attach_string",
        ) as log_attach_string:
            log_attach(entry=entry, key="foo", value="bar")

        log_attach_string.assert_called_once_with(
            entry,
            "foo",
            "bar",
        )

    def test_attach_boolean_value(self):
        entry = MagicMock()

        with patch(
            "pyfreelan.api.log.native.freelan_log_attach_boolean",
        ) as log_attach_boolean:
            log_attach(entry=entry, key="foo", value=True)

        log_attach_boolean.assert_called_once_with(
            entry,
            "foo",
            True,
        )

    def test_attach_integer_value(self):
        entry = MagicMock()

        with patch(
            "pyfreelan.api.log.native.freelan_log_attach_integer",
        ) as log_attach_integer:
            log_attach(entry=entry, key="foo", value=42)

        log_attach_integer.assert_called_once_with(
            entry,
            "foo",
            42,
        )

    def test_attach_float_value(self):
        entry = MagicMock()

        with patch(
            "pyfreelan.api.log.native.freelan_log_attach_float",
        ) as log_attach_float:
            log_attach(entry=entry, key="foo", value=3.14)

        log_attach_float.assert_called_once_with(
            entry,
            "foo",
            3.14,
        )

    def test_attach_invalid_value(self):
        entry = MagicMock()

        with self.assertRaises(TypeError):
            log_attach(entry=entry, key="foo", value=MagicMock())


class LogTests(TestCase):
    def test_log_no_file_no_payload(self):
        with patch(
            "pyfreelan.api.log.native.freelan_log",
            return_value=True,
        ) as _log:
            result = log(LogLevel.information, 3.14, LogDomain.generic, "mycode")

        _log.called_once_with(
            LogLevel.information,
            3.14,
            LogDomain.generic,
            "mycode",
            0,
            ffi.NULL,
            ffi.NULL,
            0,
        )
        self.assertTrue(result)

    def test_log_file_no_payload(self):
        with patch(
            "pyfreelan.api.log.native.freelan_log",
            return_value=False,
        ) as _log:
            result = log(
                LogLevel.information,
                3.14,
                LogDomain.generic,
                "mycode",
                file="myfile",
                line=123,
            )

        _log.called_once_with(
            LogLevel.information,
            3.14,
            LogDomain.generic,
            "mycode",
            0,
            ffi.NULL,
            "myfile",
            123,
        )
        self.assertFalse(result)

    def test_log_payload(self):
        mock_log_start = MagicMock()
        mock_log_attach = MagicMock()
        mock_log_complete = MagicMock(return_value=True)
        patcher = patch(
            "pyfreelan.api.log.native.freelan_log_start",
            mock_log_start,
        )
        patcher.start()
        self.addCleanup(patcher.stop)

        patcher = patch(
            "pyfreelan.api.log.native.freelan_log_complete",
            mock_log_complete,
        )
        patcher.start()
        self.addCleanup(patcher.stop)

        patcher = patch(
            "pyfreelan.api.log.log_attach",
            mock_log_attach,
        )
        patcher.start()
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
            3.14,
            LogDomain.generic,
            "mycode",
            payload=payload,
        )

        mock_log_start.assert_called_once_with(
            LogLevel.information,
            3.14,
            LogDomain.generic,
            "mycode",
            ffi.NULL,
            0,
        )
        entry = mock_log_start.return_value
        self.assertEqual(
            [
                call(entry, key, value)
                for key, value in payload.iteritems()
            ],
            mock_log_attach.mock_calls,
        )
        mock_log_complete.assert_called_once_with(entry)
        self.assertTrue(result)
