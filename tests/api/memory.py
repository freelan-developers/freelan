"""
Types API tests.
"""

from .. import MemoryTests

from pyfreelan.api import native, ffi
from contextlib import contextmanager


class APIMemoryTests(MemoryTests):
    def test_malloc(self):
        result = self.smartptr(
            native.freelan_malloc(6, ffi.NULL, 0),
            native.freelan_free,
        )
        self.assertNotEqual(ffi.NULL, result)

    def test_realloc(self):
        buf = self.smartptr(
            native.freelan_malloc(6, ffi.NULL, 0),
            native.freelan_free,
        )
        result = native.freelan_realloc(buf, 12, ffi.NULL, 0)

        self.assertNotEqual(ffi.NULL, result)

        if result != buf:
            self.disown(buf)
            self.smartptr(
                result,
                native.freelan_free,
            )

    def test_strdup(self):
        result = self.smartptr(
            native.freelan_strdup("freelan"),
            native.freelan_free,
        )
        self.assertEqual("freelan", ffi.string(result))
