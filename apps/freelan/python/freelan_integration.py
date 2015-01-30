"""
The main Python script called by FreeLAN.
"""

from freelan import (
    Core,
    LOGGER,
)


def main():
    """
    Entry-point.
    """
    try:
        from pyfreelan import run
    except ImportError:
        LOGGER.fatal('Unable to find `pyfreelan`. You can install it with `pip install pyfreelan` in the Python environment.')
    else:
        run(core=Core())
