"""
Exceptions classes.
"""

from tabulate import tabulate


class MemoryLeak(RuntimeError):
    """Indicates that a unit test leaks memory."""

    def __init__(self, leaks, sequence):
        super(MemoryLeak, self).__init__()
        self.leaks = leaks
        self.sequence = sequence

    def __str__(self):
        return "\n\nLeaks\n\n%s\n\nSequence\n\n%s" % (
            tabulate(
                [
                    (
                        pi.pointer,
                        pi.size,
                        pi.file,
                        pi.line,
                    )
                    for pi in self.leaks
                ],
                headers=[
                    "Address",
                    "Size (bytes)",
                    "File",
                    "Line",
                ],
                tablefmt='grid',
            ),
            tabulate(
                [
                    [
                        entry.action,
                        entry.ptrinfo.pointer,
                        entry.ptrinfo.size,
                        entry.ptrinfo.file,
                        entry.ptrinfo.line,
                    ]
                    for entry in self.sequence
                ],
                headers=[
                    "Action",
                    "Address",
                    "Size (bytes)",
                    "File",
                    "Line",
                ],
                tablefmt='grid',
            ),
        )
