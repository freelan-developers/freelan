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
    void* freelan_malloc(size_t size);
    void* freelan_realloc(void* ptr, size_t size);
    void freelan_free(void* ptr);
    char* freelan_strdup(const char* str);
    void freelan_register_memory_functions(void* (*malloc_func)(size_t), void* (*realloc_func)(void*, size_t), void (*free_func)(void*), char* (*strdup_func)(const char*));
    void* freelan_mark_pointer(void* ptr, const char* file, unsigned int line);
    void freelan_register_memory_debug_functions(void* (*mark_pointer_func)(void*, const char*, unsigned int));

    /* Error */
    struct ErrorContext;
    struct ErrorContext* freelan_acquire_error_context(void);
    void freelan_release_error_context(struct ErrorContext* ectx);
    void freelan_error_context_reset(struct ErrorContext* ectx);
    const char* freelan_error_context_get_error_category(const struct ErrorContext* ectx);
    int freelan_error_context_get_error_code(const struct ErrorContext* ectx);
    const char* freelan_error_context_get_error_description(const struct ErrorContext* ectx);
    const char* freelan_error_context_get_error_file(const struct ErrorContext* ectx);
    unsigned int freelan_error_context_get_error_line(const struct ErrorContext* ectx);

    /* Types */
    struct IPv4Address;
    struct IPv4Address* freelan_IPv4Address_from_string(const struct ErrorContext* ectx, const char* str);
    char* freelan_IPv4Address_to_string(const struct ErrorContext* ectx, struct IPv4Address* inst);
    void freelan_IPv4Address_free(struct IPv4Address* inst);
"""

ffi.cdef(api)

native = ffi.verify(
    source=api,
    libraries=['freelan'],
    include_dirs=['./include'],
)


import threading

from functools import wraps


class ErrorContext(object):
    """
    Wraps the error context logic.
    """

    @classmethod
    def get_current(cls):
        data = threading.local()

        if not hasattr(data, 'freelan_error_context'):
            data.freelan_error_context = cls()

        return data.freelan_error_context

    def __init__(self):
        self._opaque_ptr = native.freelan_acquire_error_context()

    def __del__(self):
        native.freelan_release_error_context(self._opaque_ptr)
        self._opaque_ptr = None

    def reset(self):
        native.freelan_error_context_reset(self._opaque_ptr)

    @property
    def category(self):
        value = native.freelan_error_context_get_error_category(self._opaque_ptr)

        if value != ffi.NULL:
            return ffi.string(value)

    @property
    def code(self):
        return native.freelan_error_context_get_error_code(self._opaque_ptr)

    @property
    def description(self):
        value = native.freelan_error_context_get_error_description(self._opaque_ptr)

        if value != ffi.NULL:
            return ffi.string(value)

    @property
    def file(self):
        value = native.freelan_error_context_get_error_file(self._opaque_ptr)

        if value != ffi.NULL:
            return ffi.string(value)

    @property
    def line(self):
        return native.freelan_error_context_get_error_line(self._opaque_ptr)

    def __bool__(self):
        return self.category is not None

    def __enter__(self):
        self.reset()
        return self._opaque_ptr

    def __exit__(self, type, value, traceback):
        self.raise_for_error()
        return False

    def raise_for_error(self):
        if not self:
            raise FreeLANException(error_context=self)


class FreeLANException(RuntimeError):
    def __init__(self, error_context):
        super(FreeLANException, self).__init__()
        self.error_context = error_context

    def __str__(self):
        if self:
            file, line = self.error_context.file, self.error_context.line
            file_line_suffix = " from {file}:{line}".format(
                file=file,
                line=line,
            ) if file and line else ""

            return (
                "{ectx.category}-{ectx.code}: "
                "{ectx.description}{suffix}"
            ).format(
                ectx=self.error_context,
                suffix=file_line_suffix,
            )


def check_error_context(func):
    @wraps(func)
    def wrapper(*args, **kwargs):
        with ErrorContext.get_current() as ectx:
            kwargs['ectx'] = ectx
            return func(*args, **kwargs)

    return wrapper


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
        @check_error_context
        def __init__(self, _str, ectx):
            """
            Create an instance from its string representation.

            :param _str: The string representation.
            """
            self._opaque_ptr = from_string(ectx, _str)

        def __del__(self):
            free(self._opaque_ptr)
            self._opaque_ptr = None

        @check_error_context
        def __str__(self, ectx):
            """
            Get the string representation of.

            :returns: The string representation.
            """
            value_ptr = to_string(ectx, self._opaque_ptr)
            value = ffi.string(value_ptr)
            native.freelan_free(value_ptr)

            return value

        def __repr__(self):
            return '{classname}({ptr})'.format(
                classname=self.__class__.__name__,
                ptr=self._opaque_ptr,
            )

    return Wrapper
