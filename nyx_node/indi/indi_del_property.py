# -*- coding: utf-8 -*-
########################################################################################################################
# NyxNode
# Author: Jérôme ODIER <jerome.odier@lpsc.in2p3.fr>
# SPDX-License-Identifier: GPL-3.0+
########################################################################################################################

from .. import bind
from .. import obj

########################################################################################################################

def nyx_del_property_new(device: str, name: str | None = None, message: str | None = None) -> obj.NyxObject:

    return obj.NyxObject(bind.lib.nyx_del_property_new(
        bind.as_bytes(device, allow_none = False),
        bind.as_bytes(name),
        bind.as_bytes(message),
    ))

########################################################################################################################

__all__ = [name for name in globals() if name.lower().startswith('nyx')]

########################################################################################################################
