from unittest import TestCase

from pyfreelan.api import native


class NativeTests(TestCase):
    @classmethod
    def setUpClass(cls):
        super(NativeTests, cls).setUpClass()
        cls.ectx = native.freelan_acquire_error_context()

    @classmethod
    def tearDownClass(cls):
        native.freelan_release_error_context(cls.ectx)
        cls.ectx = None
        super(NativeTests, cls).tearDownClass()

from .api import *  # noqa
