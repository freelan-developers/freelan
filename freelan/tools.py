"""A set of tools functions."""

def unique(seq):
    seen = set()
    for item in seq:
        if item not in seen:
            seen.add(item)
            yield item

def is_32_bits_architecture(arch):
    """Check if the specified architecture is 32 bits."""
    return arch in ['x86', '32', 'i386', 'i486', 'i586', 'i686', 'win32']

def is_64_bits_architecture(arch):
    """Check if the specified architecture is 64 bits."""
    return arch in ['x64', '64', 'x86_64', 'amd64', 'AMD64']
