# -*- coding: utf-8 -*-
########################################################################################################################
# NyxNode
# Author: Jérôme ODIER <jerome.odier@lpsc.in2p3.fr>
# SPDX-License-Identifier: GPL-3.0+
########################################################################################################################

import ctypes
import typing

########################################################################################################################

from .. import bind
from ..json import json_dict
from . import helpers

########################################################################################################################

class NyxSwitchProp(json_dict.NyxDict):

    ####################################################################################################################

    def __init__(self, name: str, label: str | None = None, value: helpers.NyxOnOff | int | str | bool = helpers.NyxOnOff.NYX_ONOFF_OFF):

        super().__init__(bind.lib.nyx_switch_prop_new(
            bind.as_bytes(name, allow_none = False),
            bind.as_bytes(label),
            helpers.nyx_onoff(value),
        ))

    ####################################################################################################################

    def get_value(self) -> helpers.NyxOnOff:

        return helpers.nyx_onoff(self['$'].get())

    ####################################################################################################################

    def set_value(self, value: helpers.NyxOnOff | int | str | bool) -> bool:

        return self['$'].set(helpers.nyx_onoff_str(value))

########################################################################################################################

class NyxSwitchVector(json_dict.NyxDict):

    ####################################################################################################################

    def __init__(self, device: str, name: str, state: helpers.NyxState | int | str, perm: helpers.NyxPerm | int | str, rule: helpers.NyxRule | int | str, props: typing.Iterable[NyxSwitchProp], **opts: typing.Any):

        ################################################################################################################

        opts_p = bind.as_opts(opts)

        ################################################################################################################

        super().__init__(bind.lib.nyx_switch_vector_new(
            bind.as_bytes(device, allow_none = False),
            bind.as_bytes(name, allow_none = False),
            helpers.nyx_state(state),
            helpers.nyx_perm(perm),
            helpers.nyx_rule(rule),
            ctypes.c_void_p(),
            opts_p,
        ))

        ################################################################################################################

        children = self['children']

        for prop in props:

            children.push(prop)

########################################################################################################################

__all__ = [name for name in globals() if name.lower().startswith('nyx')]

########################################################################################################################
