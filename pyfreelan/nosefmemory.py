from nose.plugins import Plugin
from nose.plugins.errorclass import ErrorClass, ErrorClassPlugin

from tabulate import tabulate

from .api import (
    memory_map,
    memory_usage,
    memory_sequence,
)


class FreeLANMemoryStats(Plugin):
    name = 'flmemstats'
    score = 1

    def report(self, stream):
        if self.enabled:
            stream.writeln("Memory report")
            stream.writeln("")
            stream.writeln(tabulate(
                [
                    ["Attribute", "Value"],
                    ["Allocations", memory_usage['allocs']],
                    ["Reallocations", memory_usage['reallocs']],
                    ["Deallocations", memory_usage['deallocs']],
                    ["Current memory usage", memory_usage['current']],
                    ["Maximum memory usage", memory_usage['max']],
                    ["Cumulated memory usage", memory_usage['sum']],
                ],
                headers="firstrow",
                tablefmt='grid',
            ))


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


class FreeLANMemoryLeak(ErrorClassPlugin):
    name = 'flmemleak'
    score = 1
    leaking = ErrorClass(MemoryLeak, label='LEAKING', isfailure=True)

    def startTest(self, test):
        self.memory_map = memory_map.copy()
        self.memory_sequence_offset = len(memory_sequence)

    def stopTest(self, test):
        if memory_map != self.memory_map:
            leaks = {
                ptrinfo
                for ptr, ptrinfo in memory_map.iteritems()
                if ptr not in self.memory_map
            }
            sequence = memory_sequence[self.memory_sequence_offset:]
            raise MemoryLeak(leaks, sequence)

    def formatError(self, test, err):
        if isinstance(err[1], MemoryLeak):
            return (err[0], err[1], None)
        else:
            return err
