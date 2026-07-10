# -*- coding: utf-8 -*-
########################################################################################################################
# NyxNode
# Author: Jérôme ODIER <jerome.odier@lpsc.in2p3.fr>
# SPDX-License-Identifier: GPL-3.0+
########################################################################################################################

import enum

########################################################################################################################

from . import bind

########################################################################################################################

## @defgroup LOGGER_PY Nyx logger
#  @brief Nyx logger.

########################################################################################################################

class NyxLogLevel(enum.IntEnum):
    """!
    @ingroup LOGGER_PY
    @brief Nyx log levels.

    | Symbol              | Value | Description       |
    | :------------------ | :---- | :---------------- |
    | `NyxLogLevel.NONE`  | 100   | Logging disabled. |
    | `NyxLogLevel.FATAL` | 101   | Fatal level.      |
    | `NyxLogLevel.ERROR` | 102   | Error level.      |
    | `NyxLogLevel.WARN`  | 103   | Warning level.    |
    | `NyxLogLevel.INFO`  | 104   | Log level.        |
    | `NyxLogLevel.DEBUG` | 105   | Debug level.      |
    | `NyxLogLevel.TRACE` | 106   | Trace level.      |
    """

    NONE  = 100
    FATAL = 101
    ERROR = 102
    WARN  = 103
    INFO  = 104
    DEBUG = 105
    TRACE = 106

########################################################################################################################

def nyx_set_log_level(level: NyxLogLevel) -> None:
    """!
    @ingroup LOGGER_PY
    @brief Sets the log level threshold.

    @param level Log level threshold.
    @return None
    """

    if not isinstance(level, NyxLogLevel):

        raise TypeError('Expected NyxLogLevel enum')

    bind.lib.nyx_set_log_level(level)

########################################################################################################################

__all__ = [name for name in globals() if name.lower().startswith('nyx')]

########################################################################################################################
