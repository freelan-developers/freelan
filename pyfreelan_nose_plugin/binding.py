"""
The binding between FreeLAN's memory function and the nose plugin.
"""

import logging

from traceback import extract_stack
from struct import calcsize

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
memory_blocks = {}
memory_blocks_offset = 0
logger = logging.getLogger("pyfreelan_nose_plugin")


def get_pointer_size():
    return calcsize('P')


def pointer_to_address(pointer):
    from pyfreelan.api import ffi

    format_string = "0x%0" + str(get_pointer_size()) + "x"
    return format_string % int(ffi.cast('unsigned long', pointer))


def pointer_to_type(pointer):
    from pyfreelan.api import ffi

    return ffi.typeof(pointer).cname


class PointerInfo(object):

    def __init__(self, pointer, size, stack):
        self.pointer = pointer
        self.size = size
        self._file = None
        self._line = None
        self.stack = stack

    def mark_pointer(self, file, line):
        self._file = file
        self._line = line

    @property
    def file(self):
        from pyfreelan.api import ffi

        if self._file is None:
            return "<no file information>"
        elif self._file == ffi.NULL:
            return "<unknown file>"
        else:
            return ffi.string(self._file).decode('utf-8')

    @property
    def line(self):
        if self._line is None:
            return "<no line information>"
        elif not self._line:
            return "<unknown line>"
        else:
            return self._line

    @property
    def python_file(self):
        return self.stack[-1][0]

    @property
    def python_line(self):
        return self.stack[-1][1]

    @property
    def python_function(self):
        return self.stack[-1][2]

    @property
    def python_code(self):
        return self.stack[-1][3]

    @property
    def has_debug_info(self):
        return self._line is not None or self._file is not None

    def __repr__(self):
        return (
            "{self.pointer} ({self.size} bytes) allocated at "
            "{self.file}:{self.line} from {self.python_file}:"
            "{self.python_line} in {self.python_function} (at "
            "'{self.python_code}')".format(self=self)
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

    @property
    def ptrinfo(self):
        return self.new_ptrinfo


callbacks = []


def save(func):
    callbacks.append(func)
    return func


def register_memory_functions():
    """
    Instructs libfreelan to use the Python memory functions.

    Use only for debugging as it has a huge performance cost.
    """
    from pyfreelan.api import (
        native,
        ffi,
    )

    @save
    @ffi.callback("void (void*, size_t)")
    def on_malloc(ptr, size):
        global memory_blocks_offset

        ptrinfo = PointerInfo(ptr, size, extract_stack()[:-2])
        allocation = Allocation(ptrinfo)
        memory_sequence.append(allocation)
        memory_map[ptr] = ptrinfo
        memory_usage['sum'] += size
        memory_usage['current'] += size
        memory_usage['max'] = max(memory_usage['max'], memory_usage['current'])
        memory_usage['allocs'] += 1
        block = memory_blocks[ptr] = memory_blocks_offset
        memory_blocks_offset += 1

        logger.info("[%3d] %r", block, allocation)

    @save
    @ffi.callback("void (void*, void*, size_t)")
    def on_realloc(old_ptr, ptr, size):
        old_ptrinfo = memory_map[old_ptr]
        new_ptrinfo = PointerInfo(ptr, size, extract_stack()[:-2])
        reallocation = Reallocation(old_ptrinfo, new_ptrinfo)
        memory_sequence.append(reallocation)

        if ptr != ffi.NULL:
            del memory_map[old_ptr]
            memory_map[ptr] = new_ptrinfo
            memory_usage['sum'] += size
            memory_usage['current'] += (size - old_ptrinfo.size)
            memory_usage['max'] = max(
                memory_usage['max'],
                memory_usage['current'],
            )
            memory_usage['reallocs'] += 1

            block = memory_blocks[ptr] = memory_blocks.pop(old_ptr)

            logger.info("[%3d] %r", block, reallocation)
        else:
            logger.warning("Reallocation returned a null pointer (%r)", ptr)

    @save
    @ffi.callback("void (void*)")
    def on_free(ptr):
        if ptr != ffi.NULL:
            ptrinfo = memory_map[ptr]
            deallocation = Deallocation(ptrinfo)
            memory_sequence.append(deallocation)
            memory_usage['deallocs'] += 1
            memory_usage['current'] -= memory_map[ptr].size
            del memory_map[ptr]
            block = memory_blocks.pop(ptr)

            logger.info("[%3d] %r", block, deallocation)
        else:
            logger.info("Deallocating null pointer (%r)", ptr)

    @save
    @ffi.callback("void* (void*, const char*, unsigned int)")
    def mark_pointer(ptr, file, line):
        memory_map[ptr].mark_pointer(file, line)
        block = memory_blocks[ptr]
        ptr_info = memory_map[ptr]
        logger.info("[%3d] Added pointer information to: %r", block, ptr_info)

        return ptr

    native.freelan_register_memory_debug_functions(
        on_malloc,
        on_realloc,
        mark_pointer,
        on_free,
    )


def unregister_memory_functions():
    """
    Instructs libfreelan to use the default memory functions.
    """
    from pyfreelan.api import (
        native,
        ffi,
    )

    native.freelan_register_memory_debug_functions(
        ffi.NULL,
        ffi.NULL,
        ffi.NULL,
        ffi.NULL,
    )
    callbacks[:] = []


def cleanup_memory_cache():
    """
    Cleans memory caches to complete malloc-free pending cycles.

    Mainly used to ensure thread-local error context are destroyed.
    """
    from pyfreelan.api.error import ErrorContext

    ErrorContext.clear_current()


def enable_memory_logs():
    """
    Enable the memory allocation/deallocation logs.
    """
    logger.setLevel(logging.INFO)
