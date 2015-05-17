"""
Exceptions classes.
"""

from tabulate import tabulate

from .binding import (
    pointer_to_address,
    pointer_to_type,
)


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
                        pointer_to_address(pi.pointer),
                        pointer_to_type(pi.pointer),
                        pi.size,
                        pi.file,
                        pi.line,
                    )
                    for pi in self.leaks
                ],
                headers=[
                    "Address",
                    "Type",
                    "Size (bytes)",
                    "File",
                    "Line",
                ],
                tablefmt='plain',
            ),
            tabulate(
                [
                    [
                        "*" if entry.ptrinfo in self.leaks else "",
                        entry.action,
                        pointer_to_address(entry.ptrinfo.pointer),
                        pointer_to_type(entry.ptrinfo.pointer),
                        entry.ptrinfo.size,
                        entry.ptrinfo.file,
                        entry.ptrinfo.line,
                    ]
                    for entry in self.sequence
                ],
                headers=[
                    "Leaking",
                    "Action",
                    "Address",
                    "Type",
                    "Size (bytes)",
                    "File",
                    "Line",
                ],
                tablefmt='plain',
            ),
        )
