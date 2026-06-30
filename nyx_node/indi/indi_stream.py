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
from .. import json

from . import enums
from . import utils

########################################################################################################################

@utils.nyx_property(
    'name',
    '@name',
)
@utils.nyx_property(
    'label',
    '@label',
)
class NyxStreamProp(json.json_dict.NyxDict):

    ####################################################################################################################

    def __init__(self, name: str, label: str | None = None):

        super().__init__(bind.lib.nyx_stream_prop_new(
            bind.as_bytes(name, allow_none = False),
            bind.as_bytes(label, allow_none = True),
        ))

########################################################################################################################

@utils.nyx_property(
    'device',
    '@device',
)
@utils.nyx_property(
    'name',
    '@name',
)
@utils.nyx_property(
    'state',
    '@state',
    getter = enums.nyx_state,
    setter = enums.nyx_state_str,
)
class NyxStreamVector(json.json_dict.NyxDict):

    ####################################################################################################################

    def __init__(self, device: str, name: str, state: enums.NyxState | int | str, props: typing.Iterable[NyxStreamProp], **opts: typing.Any):

        ################################################################################################################

        super().__init__(bind.lib.nyx_stream_vector_new(
            bind.as_bytes(device, allow_none = False),
            bind.as_bytes(name, allow_none = False),
            enums.nyx_state(state),
            ctypes.c_void_p(),
            bind.as_opts(opts),
        ))

        ################################################################################################################

        # noinspection PyTypeChecker
        children: json.NyxList = self['children']

        for prop in props:

            if not isinstance(prop, NyxStreamProp):

                raise TypeError(f'Expected NyxStreamProp')

            children.push(prop)

########################################################################################################################

__all__ = [name for name in globals() if name.lower().startswith('nyx')]

########################################################################################################################
