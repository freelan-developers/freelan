"""
Test the API version functions and helpers.
"""

from unittest import TestCase
from mock import patch

from pyfreelan.api.version import (
    get_version_string,
    get_version,
)


class VersionTests(TestCase):

    def test_get_version_string(self):
        with patch(
            "pyfreelan.api.version.native.freelan_get_version_string",
        ) as get_version_string_mock:
            with patch("pyfreelan.api.version.ffi.string") as string_mock:
                result = get_version_string()

        get_version_string_mock.assert_called_once_with()
        string_mock.assert_called_once_with(get_version_string_mock())
        self.assertEqual(string_mock(get_version_string_mock()), result)

    def test_get_version(self):
        with patch(
            "pyfreelan.api.version.get_version_string",
            return_value="1.2.3",
        ):
            result = get_version()

        self.assertEqual((1, 2, 3), result)
