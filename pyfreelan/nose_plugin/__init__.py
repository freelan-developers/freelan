from nose.plugins.errorclass import ErrorClass, ErrorClassPlugin

from tabulate import tabulate

from .binding import (
    register_memory_functions,
    unregister_memory_functions,
    memory_map,
    memory_usage,
    memory_sequence,
)
from .exceptions import MemoryLeak


class FreeLANMemory(ErrorClassPlugin):
    name = 'flmem'
    score = 1
    enabled = True
    leaking = ErrorClass(MemoryLeak, label='LEAKING', isfailure=True)

    def options(self, parser, env):
        parser.add_option(
            '--mem-report',
            action='store_true',
            default=False,
            dest='memReport',
            help="Enable FreeLAN memory report.",
        )
        parser.add_option(
            '--mem-leaks',
            action='store_true',
            default=False,
            dest='memLeaks',
            help="Enable FreeLAN memory leaks analysis.",
        )

    def configure(self, options, conf):
        if not self.can_configure:
            return

        self.conf = conf
        self.memReport = getattr(options, 'memReport', False)
        self.memLeaks = getattr(options, 'memLeaks', False)

    def report(self, stream):
        if self.memReport:
            stream.writeln("-" * 70)
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

    def begin(self):
        if self.memLeaks:
            register_memory_functions()

    def finalize(self, result):
        if self.memLeaks:
            unregister_memory_functions()

    def startTest(self, test):
        if self.memLeaks:
            self.memory_map = memory_map.copy()
            self.memory_sequence_offset = len(memory_sequence)

    def stopTest(self, test):
        if self.memLeaks:
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
