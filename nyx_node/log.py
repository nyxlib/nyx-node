# -*- coding: utf-8 -*-
########################################################################################################################
# NyxNode
# Author: Jérôme ODIER <jerome.odier@lpsc.in2p3.fr>
# SPDX-License-Identifier: GPL-3.0+
########################################################################################################################

from . import bind

########################################################################################################################

NyxLogLevel = bind.NyxLogLevel

########################################################################################################################

def nyx_set_log_level(level: bind.NyxLogLevel) -> None:
    """! @brief Sets the log level threshold.

    @param level Log level threshold.
    """

    if not isinstance(level, NyxLogLevel):

        raise TypeError('Expected NyxLogLevel enum')

    bind.lib.nyx_set_log_level(level)

########################################################################################################################

__all__ = [name for name in globals() if name.lower().startswith('nyx')]

########################################################################################################################
