from . import (
    native,
    ffi,
)


def get_version_string():
    """
    Get the FreeLAN version string.

    :returns: The version string.
    """
    return ffi.string(native.freelan_get_version_string())


def get_version():
    """
    Get the FreeLAN version, as a tuple.

    :returns: A tuple (major, minor, patch).
    """
    return tuple(map(int, get_version_string().split(".")))
