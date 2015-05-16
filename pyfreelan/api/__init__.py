"""
FreeLAN API.
"""

import cffi

ffi = cffi.FFI()

api = """
    /* Misc */
    void* malloc(size_t size);
    void* realloc(void* ptr, size_t size);
    void free(void* ptr);

    /* Memory */
    void* freelan_malloc(size_t size, const char*, unsigned int);
    void* freelan_realloc(void* ptr, size_t size, const char*, unsigned int);
    void freelan_free(void* ptr);
    char* freelan_strdup(const char* str);
    void freelan_register_memory_functions(void* (*malloc_func)(size_t, const char*, unsigned int), void* (*realloc_func)(void*, size_t, const char*, unsigned int), void (*free_func)(void*), char* (*strdup_func)(const char*));

    /* Types */
    struct IPv4Address;
    struct IPv4Address* freelan_IPv4Address_from_string(const char* str);
    char* freelan_IPv4Address_to_string(struct IPv4Address* inst);
    void freelan_IPv4Address_free(struct IPv4Address* inst);
"""

ffi.cdef(api)

native = ffi.verify(
    source=api,
    libraries=['freelan'],
    include_dirs=['./include'],
)

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


class PointerInfo(object):
    def __init__(self, pointer, size, file, line):
        self.pointer = pointer
        self.size = size
        self._file = file
        self.line = line

    @property
    def file(self):
        if self._file == ffi.NULL:
            return "<unknown file>"
        else:
            return ffi.string(self._file)

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


@ffi.callback("void* (size_t, const char*, unsigned int)")
def malloc(size, file, line):
    result = native.malloc(size)
    ptrinfo = PointerInfo(result, size, file, line)
    memory_sequence.append(Allocation(ptrinfo))
    memory_map[result] = ptrinfo
    memory_usage['sum'] += size
    memory_usage['current'] += size
    memory_usage['max'] = max(memory_usage['max'], memory_usage['current'])
    memory_usage['allocs'] += 1

    return result


@ffi.callback("void* (void*, size_t, const char*, unsigned int)")
def realloc(ptr, size, file, line):
    result = native.realloc(ptr, size)
    old_ptrinfo = memory_map[ptr]
    new_ptrinfo = PointerInfo(result, size, file, line)
    memory_sequence.append(Reallocation(old_ptrinfo, new_ptrinfo))

    if result != ffi.NULL:
        del memory_map[ptr]
        memory_map[result] = new_ptrinfo
        memory_usage['sum'] += size
        memory_usage['current'] += (size - old_ptrinfo.size)
        memory_usage['reallocs'] += 1

    return result


@ffi.callback("void (void*)")
def free(ptr):
    ptrinfo = memory_map[ptr]
    memory_sequence.append(Deallocation(ptrinfo))
    result = native.free(ptr)
    memory_usage['deallocs'] += 1
    memory_usage['current'] -= memory_map[ptr].size
    del memory_map[ptr]

    return result


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


def unregister_memory_functions():
    """
    Instructs libfreelan to use the default memory functions.
    """
    native.freelan_register_memory_functions(
        ffi.NULL,
        ffi.NULL,
        ffi.NULL,
        ffi.NULL,
    )


def NativeType(typename):
    """
    Create a API wrapper base class around the specified native type.

    :param typename: The name of the native type to wrap.
    :returns: An API wrapper class.
    """
    from_string = getattr(native, 'freelan_%s_from_string' % typename)
    to_string = getattr(native, 'freelan_%s_to_string' % typename)
    free = getattr(native, 'freelan_%s_free' % typename)

    class Wrapper(object):
        def __init__(self, _str):
            """
            Create an instance from its string representation.

            :param _str: The string representation.
            """
            self._opaque_ptr = from_string(_str)

        def __del__(self):
            free(self._opaque_ptr)
            self._opaque_ptr = None

        def __str__(self):
            """
            Get the string representation of.

            :returns: The string representation.
            """
            value_ptr = to_string(self._opaque_ptr)
            value = ffi.string(value_ptr)
            native.freelan_free(value_ptr)

            return value

        def __repr__(self):
            return '{classname}({ptr})'.format(
                classname=self.__class__.__name__,
                ptr=self._opaque_ptr,
            )

    return Wrapper
