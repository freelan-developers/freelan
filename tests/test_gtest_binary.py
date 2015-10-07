"""
Python wrapper that runs the gtest binary.
"""

from __future__ import print_function
from __future__ import unicode_literals

from subprocess import (
    Popen,
    PIPE,
)

GTEST_BINARY = './install/bin/freelan_tests'


class TestGTestBinary(object):

    def test_run_gtest_binary(self):
        try:
            process = Popen(
                [GTEST_BINARY, '--gtest_list_tests'],
                stdout=PIPE,
            )
            lines = iter(process.stdout.readline, b'')

            # Skip the first line as it contains garbage.
            next(lines)

            module = None
            name = None

            for line in lines:
                if line.startswith(b'  '):
                    name = line.strip()
                else:
                    module = line.strip().rstrip(b'.')

                if name and module:
                    yield self.run_single, module, name

        except OSError:
            assert False, (
                "Unable to find the gtest binary at: %s" % GTEST_BINARY
            )

    def run_single(self, module, name):
        process = Popen(
            [GTEST_BINARY, '--gtest_filter=%s.%s' % (module, name)],
            stdout=PIPE,
        )
        print(process.stdout.read())
        process.wait()
        assert 0 == process.returncode
