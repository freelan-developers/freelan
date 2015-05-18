"""
The binding between FreeLAN's memory function and the nose plugin.
"""

from struct import calcsize

from pyfreelan.api import (
    native,
    ffi,
)
from pyfreelan.api.error import ErrorContext

memory_map = {}
memory_usage = {
    'current': 0,
    'max': 0,
    'sum': 0,
    'allocs': 0,
    'reallocs': 0,
    'deallocs': 0,
}
memory_sequence = []


def get_pointer_size():
    return calcsize('P')


def pointer_to_address(pointer):
    format_string = "0x%0" + str(get_pointer_size()) + "x"
    return format_string % int(ffi.cast('unsigned long', pointer))


def pointer_to_type(pointer):
    return ffi.typeof(pointer).cname


class PointerInfo(object):
    def __init__(self, pointer, size):
        self.pointer = pointer
        self.size = size
        self._file = None
        self._line = None

    def mark_pointer(self, file, line):
        self._file = file
        self._line = line

    @property
    def file(self):
        if self._file is None:
            return "<no file information>"
        elif self._file == ffi.NULL:
            return "<unknown file>"
        else:
            return ffi.string(self._file)

    @property
    def line(self):
        if self._line is None:
            return "<no line information>"
        elif not self._line:
            return "<unknown line>"
        else:
            return self._line

    @property
    def has_debug_info(self):
        return self._line is not None or self._file is not None

    def __repr__(self):
        return (
            "{self.pointer} ({self.size} bytes) allocated at "
            "{self.file}:{self.line}".format(self=self)
        )


class Allocation(object):
    def __init__(self, ptrinfo):
        self.ptrinfo = ptrinfo
        self.action = "malloc"

    def __repr__(self):
        return "Allocation({self.ptrinfo})".format(self=self)


class Deallocation(object):
    def __init__(self, ptrinfo):
        self.ptrinfo = ptrinfo
        self.action = "free"

    def __repr__(self):
        return "Deallocation({self.ptrinfo})".format(self=self)


class Reallocation(object):
    def __init__(self, old_ptrinfo, new_ptrinfo):
        self.old_ptrinfo = old_ptrinfo
        self.new_ptrinfo = new_ptrinfo
        self.action = "realloc"

    def __repr__(self):
        return "Reallocation({self.old_ptrinfo} => {self.new_ptrinfo})".format(
            self=self,
        )


@ffi.callback("void* (size_t)")
def malloc(size):
    result = native.malloc(size)
    ptrinfo = PointerInfo(result, size)
    memory_sequence.append(Allocation(ptrinfo))
    memory_map[result] = ptrinfo
    memory_usage['sum'] += size
    memory_usage['current'] += size
    memory_usage['max'] = max(memory_usage['max'], memory_usage['current'])
    memory_usage['allocs'] += 1

    return result


@ffi.callback("void* (void*, size_t)")
def realloc(ptr, size):
    result = native.realloc(ptr, size)
    old_ptrinfo = memory_map[ptr]
    new_ptrinfo = PointerInfo(result, size)
    memory_sequence.append(Reallocation(old_ptrinfo, new_ptrinfo))

    if result != ffi.NULL:
        del memory_map[ptr]
        memory_map[result] = new_ptrinfo
        memory_usage['sum'] += size
        memory_usage['current'] += (size - old_ptrinfo.size)
        memory_usage['max'] = max(memory_usage['max'], memory_usage['current'])
        memory_usage['reallocs'] += 1

    return result


@ffi.callback("void (void*)")
def free(ptr):
    if ptr != ffi.NULL:
        ptrinfo = memory_map[ptr]
        memory_sequence.append(Deallocation(ptrinfo))
        memory_usage['deallocs'] += 1
        memory_usage['current'] -= memory_map[ptr].size
        del memory_map[ptr]

    return native.free(ptr)


@ffi.callback("void* (void*, const char*, unsigned int)")
def mark_pointer(ptr, file, line):
    memory_map[ptr].mark_pointer(file, line)

    return ptr


def register_memory_functions():
    """
    Instructs libfreelan to use the Python memory functions.

    Use only for debugging as it has a huge performance cost.
    """
    native.freelan_register_memory_functions(
        malloc,
        realloc,
        free,
        ffi.NULL,
    )
    native.freelan_register_memory_debug_functions(
        mark_pointer,
    )


def unregister_memory_functions():
    """
    Instructs libfreelan to use the default memory functions.
    """
    native.freelan_register_memory_debug_functions(
        ffi.NULL,
    )
    native.freelan_register_memory_functions(
        ffi.NULL,
        ffi.NULL,
        ffi.NULL,
        ffi.NULL,
    )


def cleanup_memory_cache():
    """
    Cleans memory caches to complete malloc-free pending cycles.

    Mainly used to ensure thread-local error context are destroyed.
    """
    ErrorContext.clear_current()
