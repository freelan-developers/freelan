"""
FreeLAN API.
"""

import os
import cffi

ffi = cffi.FFI()

ffi.cdef(
"""
struct IPv4Address;
struct IPv4Address* freelan_IPv4Address_from_string(const char* str);
char* freelan_IPv4Address_to_string(struct IPv4Address* inst);
"""
)

native = ffi.verify('', libraries=['freelan'])
