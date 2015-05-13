from nose.plugins import Plugin


class FreeLANMemory(Plugin):
    name = 'fmemory'

    allocs = None
    reallocs = None
    deallocs = None
    max = None
    sum = None

    def finalize(self, result):
        if self.enabled:
            print """
FreeLAN memory analysis report
==============================

Allocations count: {s.allocs}
Deallocations count: {s.deallocs}
Reallocations count: {s.reallocs}
Maximum heap size: {s.max}
Total allocated bytes: {s.sum}
""".rstrip().format(s=self)

    @classmethod
    def feed_memory_information(cls, **kwargs):
        for field, value in kwargs.iteritems():
            setattr(cls, field, value)
