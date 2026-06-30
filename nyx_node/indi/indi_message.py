# -*- coding: utf-8 -*-
########################################################################################################################
# NyxNode
# Author: Jérôme ODIER <jerome.odier@lpsc.in2p3.fr>
# SPDX-License-Identifier: GPL-3.0+
########################################################################################################################

from .. import bind
from .. import json

from . import utils

########################################################################################################################

@utils.nyx_property(
    'device',
    '@device',
)
@utils.nyx_property(
    'message',
    '@message',
)
class NyxMessage(json.json_dict.NyxDict):

    ####################################################################################################################

    def __init__(self, device: str, message: str | None = None):

        super().__init__(bind.lib.nyx_message_new(
            bind.as_bytes(device, allow_none = False),
            bind.as_bytes(message, allow_none = True)
        ))

########################################################################################################################

__all__ = [name for name in globals() if name.lower().startswith('nyx')]

########################################################################################################################
