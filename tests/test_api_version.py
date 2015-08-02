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
        with patch("pyfreelan.api.version.native") as native_mock:
            with patch("pyfreelan.api.version.ffi") as ffi_mock:
                result = get_version_string()

        native_mock.freelan_get_version_string.assert_called_once_with()
        ffi_mock.string.assert_called_once_with(
            native_mock.freelan_get_version_string(),
        )
        self.assertEqual(
            ffi_mock.string(native_mock.freelan_get_version_string()),
            result,
        )

    def test_get_version(self):
        with patch(
            "pyfreelan.api.version.get_version_string",
            return_value="1.2.3",
        ):
            result = get_version()

        self.assertEqual((1, 2, 3), result)
