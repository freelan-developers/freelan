"""
Python interface between the C++ module and the outside world.
"""

import _freelan

import logging


class Core(_freelan.Core):
    """
    Represents the running FreeLAN core instance running.
    """

    def __init__(self):
        super(Core, self).__init__(_freelan._FREELAN_CORE_INSTANCE_POINTER)


class CoreLogHandler(logging.Handler):
    """
    A log handler that logs everything it gets through the FreeLAN core
    instance.
    """

    core = Core()

    def to_freelan_log_level(self, log_level):
        """
        Convert a numeric log level into its _freelan.LogLevel counterpart.

        :param log_level: The log level to convert.
        :returns: A _freelan.LogLevel instance.
        """
        if log_level < 10:
            return _freelan.LogLevel.trace
        elif log_level < 20:
            return _freelan.LogLevel.debug
        elif log_level < 25:
            return _freelan.LogLevel.information
        elif log_level < 30:
            return _freelan.LogLevel.important
        elif log_level < 40:
            return _freelan.LogLevel.warning
        elif log_level < 50:
            return _freelan.LogLevel.error
        else:
            return _freelan.LogLevel.fatal

    def emit(self, record):
        try:
            msg = self.format(record)
            self.core.log(self.to_freelan_log_level(record.levelno), msg)

        except (KeyboardInterrupt, SystemExit):
            raise

        except:
            self.handleError(record)


# Setup the FreeLAN logger.
LOGGER = logging.getLogger('freelan')
LOGGER.setLevel(logging.DEBUG)
LOGGER.addHandler(CoreLogHandler())
