from .types import *  # noqa
from .memory import *  # noqa

from . import MemoryTests

from pyfreelan.api import NativeType

from mock import patch


class NativeTypeTests(MemoryTests):
    def setUp(self):
        super(NativeTypeTests, self).setUp()

        with patch('pyfreelan.api.native') as native:
            self.cls = NativeType('Foo')
            self.native = native

    def test_instantiation(self):
        self.native.freelan_Foo_from_string.return_value = 42

        value = self.cls("myvalue")

        self.native.freelan_Foo_from_string.called_once_with("myvalue")
        self.assertEqual(42, value._opaque_ptr)
