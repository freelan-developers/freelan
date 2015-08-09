"""
IO API tests.
"""

from unittest import TestCase
from mock import MagicMock

from pyfreelan.api.io import IOService


class IOServiceTests(TestCase):

    def test_post_and_run(self):
        task = MagicMock()
        io_service = IOService()

        io_service.post(task)

        self.assertEqual([], task.mock_calls)

        io_service.run()

        task.assert_called_once_with()

    def test_post_and_run_with_user_context(self):
        task = MagicMock()
        io_service = IOService()

        io_service.post(task, 42, foo=1, bar='bar')

        self.assertEqual([], task.mock_calls)

        io_service.run()

        task.assert_called_once_with(42, foo=1, bar='bar')
