from pyfreelan.api import (
    native,
    ffi,
    register_memory_functions,
    unregister_memory_functions,
)


def setUpModule():
    register_memory_functions()


def tearDownModule():
    unregister_memory_functions()


from unittest import TestCase


class MemoryTests(TestCase):
    native = native
    ffi = ffi

    def setUp(self):
        self._deleters = {}

    def tearDown(self):
        for value, deleter in self._deleters.iteritems():
            deleter(value)

    def smartptr(self, value, deleter):
        if value != ffi.NULL:
            self.assertNotIn(value, self._deleters)
            self._deleters[value] = deleter

        return value

    def disown(self, value):
        self.assertIn(value, self._deleters)
        del self._deleters[value]


from .api import *  # noqa
