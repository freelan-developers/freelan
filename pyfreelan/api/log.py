from enum import Enum
from datetime import datetime

from . import (
    native,
    ffi,
)


class LogLevel(Enum):
    trace = native.FREELAN_LOG_LEVEL_TRACE
    debug = native.FREELAN_LOG_LEVEL_DEBUG
    information = native.FREELAN_LOG_LEVEL_INFORMATION
    important = native.FREELAN_LOG_LEVEL_IMPORTANT
    warning = native.FREELAN_LOG_LEVEL_WARNING
    error = native.FREELAN_LOG_LEVEL_ERROR
    fatal = native.FREELAN_LOG_LEVEL_FATAL


def utc_datetime_to_utc_timestamp(dt):
    """
    Converts an UTC datetime to an UTC timestamp.

    :param dt: The datetime instance to convert. Must be UTC.
    :returns: An UTC timestamp.
    """
    return (dt - datetime(1970, 1, 1)).total_seconds()


def utc_timestamp_to_utc_datetime(ts):
    """
    Converts an UTC timestamp to an UTC datetime.

    :param ts: The timestamp to convert. Must be UTC.
    :returns: An UTC datetime.
    """
    return datetime.utcfromtimestamp(ts)


def to_c(value):
    """
    Check if a value has the expected format for a call to the C API.

    :param value: The value whose format to check.
    :returns: A value suitable for a call to the C API.
    """
    if hasattr(value, 'encode'):
        value = value.encode('utf-8')

    return value


def from_c(value):
    """
    Convert value from the C API format to the local Python format, if needed.

    :param value: The value whose format to check.
    :returns: A value suitable for local use in Python.
    """
    if hasattr(value, 'decode'):
        value = value.decode('utf-8')

    return value


def log_attach(registry, entry, key, value):
    """
    Attach a value to a native log entry.

    :param key: The key. Must be a string.
    :param value: The value. Can be either a string, an integer, a float or a
    boolean value. Otherwise a TypeError is raised.

    .. note: Unicode strings are UTF-8 encoded for both keys and values.
    """
    if not isinstance(key, basestring):
        raise TypeError("key must be a string")

    key = to_c(key)
    value = to_c(value)

    if isinstance(value, str):
        # We must create a store a new const char* value so that the references
        # remains valid until entry expires.
        str_value = ffi.new("const char[]", value)
        registry.append(str_value)
        native.freelan_log_attach(entry, key, native.FREELAN_LOG_PAYLOAD_TYPE_STRING, {'as_string': str_value})
    elif isinstance(value, bool):
        native.freelan_log_attach(entry, key, native.FREELAN_LOG_PAYLOAD_TYPE_BOOLEAN, {'as_boolean': value})
    elif isinstance(value, int):
        native.freelan_log_attach(entry, key, native.FREELAN_LOG_PAYLOAD_TYPE_INTEGER, {'as_integer': value})
    elif isinstance(value, float):
        native.freelan_log_attach(entry, key, native.FREELAN_LOG_PAYLOAD_TYPE_FLOAT, {'as_float': value})
    elif value is None:
        native.freelan_log_attach(entry, key, native.FREELAN_LOG_PAYLOAD_TYPE_NULL, {'as_null': ffi.NULL})
    else:
        raise TypeError("value must be either a string, an integer, a float or a boolean value")


def log(level, domain, code, payload=None, timestamp=datetime.utcnow(), file=None, line=0):
    """
    Writes a log entry.

    :param level: The log level, as a :ref:`pyfreelan.api.log.LogLevel`
    instance.
    :param timestamp: The timestamp attached to this log entry.
    :param domain: The log domain, as a string.
    :param code: The log entry code. A code only has meaning for a given
    domain.
    :payload: A dictionary of payload values. Keys must be string and values
    must be either strings, integers, floats or boolean values.
    :param file: The file associated to the log entry. If null, ``line`` is
    ignored.
    :param line: The line associated to the log entry.
    :returns: True if the log entry was handled. A falsy return value can
    indicate that no logging callback was set or that the current log level
    does not allow the log entry to be written.
    """
    if file is None:
        file = ffi.NULL
        line = 0
    else:
        file = to_c(file)

    domain = to_c(domain)
    code = to_c(code)

    if not payload:
        return native.freelan_log(
            level.value,
            utc_datetime_to_utc_timestamp(timestamp),
            domain,
            code,
            0,
            ffi.NULL,
            file,
            line,
        ) != 0
    else:
        entry = native.freelan_log_start(
            level.value,
            utc_datetime_to_utc_timestamp(timestamp),
            domain,
            code,
            ffi.NULL if not file else file,
            line,
        )

        # This list is used to store native pointers so that they don't expire
        # until entry is deleted.
        registry = []

        try:
            for key, value in payload.iteritems():
                log_attach(registry, entry, key, value)
        finally:
            return native.freelan_log_complete(entry) != 0


CALLBACKS = {}


def set_log_function(func):
    """
    Set the log function.

    :param func: The logging function to call whenever a log entry is emitted.
    If `None`, no logging function is set.
    """
    CALLBACKS['log_function'] = func

    if func is None:
        native.freelan_set_log_function(ffi.NULL)
    else:
        native.freelan_set_log_function(c_log_function)


def from_native_payload(payload):
    """
    Return a tuple (key, value) from a native log payload.

    :param payload: The native payload to read.
    :returns: A tuple (key, value).
    """
    key = ffi.string(payload.key)

    if payload.type == native.FREELAN_LOG_PAYLOAD_TYPE_STRING:
        value = ffi.string(payload.value.as_string).decode('utf-8')
    elif payload.type == native.FREELAN_LOG_PAYLOAD_TYPE_INTEGER:
        value = payload.value.as_integer
    elif payload.type == native.FREELAN_LOG_PAYLOAD_TYPE_FLOAT:
        value = payload.value.as_float
    elif payload.type == native.FREELAN_LOG_PAYLOAD_TYPE_BOOLEAN:
        value = payload.value.as_boolean != 0
    else:
        value = None

    return key, value


def log_function(level, timestamp, domain, code, payload_size, payload, file, line):
    """
    The default logging callback.

    This callback acts as a translator from the C world to the Python realm.

    You should not need to call it directly.
    """
    log_function = CALLBACKS.get('log_function')

    if log_function:
        return 1 if log_function(
            level=LogLevel(level),
            timestamp=utc_timestamp_to_utc_datetime(timestamp),
            domain=from_c(ffi.string(domain)),
            code=from_c(ffi.string(code)),
            payload={
                key: value
                for key, value in (
                    from_native_payload(payload[i])
                    for i in xrange(payload_size)
                )
            },
            file=ffi.string(file) if file != ffi.NULL else None,
            line=line if file != ffi.NULL else 0,
        ) else 0

    return 0

c_log_function = ffi.callback(
    "int (FreeLANLogLevel, FreeLANTimestamp, char *, char *, size_t, "
    "struct FreeLANLogPayload *, char *, unsigned int)",
)(log_function)
