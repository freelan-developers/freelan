"""
IO API tests.
"""

from __future__ import unicode_literals

from six.moves import range
from unittest import TestCase
from mock import MagicMock
from threading import (
    Thread,
    Event,
)

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

    def test_threaded_runs(self):
        def wait(queue, event):
            # Wait at most 10 seconds.
            # If this test ever takes more than a second on a decent computer,
            # something is seriously wrong anyway.
            self.assertTrue(event.wait(timeout=10.0))
            queue.append("wait")

        def trigger(queue, event):
            queue.append("trigger")
            event.set()

        io_service = IOService()
        event = Event()
        queue = []

        io_service.post(wait, queue, event)
        io_service.post(trigger, queue, event)

        threads = [
            Thread(target=io_service.run)
            for _ in range(2)
        ]

        for thread in threads:
            thread.start()

        for thread in threads:
            thread.join(timeout=10.0)
            self.assertFalse(thread.is_alive())

        self.assertEqual(
            [
                "trigger",
                "wait",
            ],
            queue,
        )
