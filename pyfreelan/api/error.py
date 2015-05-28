import threading

from functools import wraps

from . import (
    native,
    ffi,
)


def convert_native_string(func):
    """
    Decorator that converts native strings in Python strings from function
    results.

    :param func: The function to decorate.
    :returns: The decorated function.
    """
    @wraps(func)
    def wrapper(*args, **kwargs):
        result = func(*args, **kwargs)

        if result != ffi.NULL:
            return ffi.string(result)

    wrapper.wrapped = func

    return wrapper


class ErrorContext(object):
    """
    Wraps the error context logic.
    """
    thread_local_data = threading.local()

    @classmethod
    def get_current(cls):
        if not hasattr(cls.thread_local_data, 'freelan_error_context'):
            cls.thread_local_data.freelan_error_context = cls()

        return cls.thread_local_data.freelan_error_context

    @classmethod
    def clear_current(cls):
        if hasattr(cls.thread_local_data, 'freelan_error_context'):
            del cls.thread_local_data.freelan_error_context

    def __init__(self):
        self._opaque_ptr = native.freelan_acquire_error_context()

    def __del__(self):
        native.freelan_release_error_context(self._opaque_ptr)
        self._opaque_ptr = None

    def reset(self):
        native.freelan_error_context_reset(self._opaque_ptr)

    @property
    @convert_native_string
    def category(self):
        return native.freelan_error_context_get_error_category(self._opaque_ptr)

    @property
    def code(self):
        return native.freelan_error_context_get_error_code(self._opaque_ptr)

    @property
    @convert_native_string
    def description(self):
        return native.freelan_error_context_get_error_description(self._opaque_ptr)

    @property
    @convert_native_string
    def file(self):
        return native.freelan_error_context_get_error_file(self._opaque_ptr)

    @property
    def line(self):
        return native.freelan_error_context_get_error_line(self._opaque_ptr)

    def __nonzero__(self):
        return self.category is not None

    def __enter__(self):
        self.reset()
        return self._opaque_ptr

    def __exit__(self, type, value, traceback):
        self.raise_for_error()

    def raise_for_error(self):
        if self:
            raise FreeLANException(error_context=self)


class FreeLANException(RuntimeError):
    def __init__(self, error_context):
        super(FreeLANException, self).__init__()
        self.error_context = error_context

    def __str__(self):
        file, line = self.error_context.file, self.error_context.line
        file_line_suffix = " ({file}:{line})".format(
            file=file,
            line=line,
        ) if file and line else ""

        return (
            "{ectx.category}:{ectx.code} - "
            "{ectx.description}{suffix}"
        ).format(
            ectx=self.error_context,
            suffix=file_line_suffix,
        )


def check_error_context(func):
    """
    Decorator that automatically adds an error context to native calls
    wrappers.

    :param func: The function to wrap. It needs to take a parameter named
    'ectx' as keyword argument.
    :returns: The wrapped function.
    """
    @wraps(func)
    def wrapper(*args, **kwargs):
        with ErrorContext.get_current() as ectx:
            kwargs['ectx'] = ectx
            return func(*args, **kwargs)

    wrapper.wrapped = func

    return wrapper
