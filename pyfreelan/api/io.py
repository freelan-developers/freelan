from __future__ import unicode_literals

from threading import Lock

from . import (
    native,
    ffi,
)

from .error import check_error_context


class IOService(object):
    """
    Handles asynchronous I/O tasks.
    """

    @check_error_context
    def __init__(self, ectx):
        self._opaque_ptr = native.freelan_IOService_new(ectx)
        self._lock = Lock()
        self._task_callbacks = {}

    def __del__(self):
        native.freelan_IOService_free(self._opaque_ptr)
        self._opaque_ptr = None

    def post(self, task, *args, **kwargs):
        @ffi.callback("void (void*)")
        def task_callback(user_ctx):
            task_id = int(ffi.cast("uintptr_t", user_ctx))

            with self._lock:
                task, args, kwargs, _ = self._task_callbacks.pop(task_id)

            task(*args, **kwargs)

        task_id = id(task_callback)

        with self._lock:
            self._task_callbacks[task_id] = (task, args, kwargs, task_callback)

        native.freelan_IOService_post(
            self._opaque_ptr,
            task_callback,
            ffi.cast("void*", task_id),
        )

    def run(self):
        native.freelan_IOService_run(self._opaque_ptr)
