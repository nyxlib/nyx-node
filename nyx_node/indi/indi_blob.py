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
@utils.nyx_property(
    'format',
    '@format',
)
@utils.nyx_property(
    'value',
    '$',
    #setter = utils.nyx_blob,
)
class NyxBlobProp(json.json_dict.NyxDict):

    ####################################################################################################################

    def __init__(self, name: str, label: str | None = None, format: str | None = None, value: bytes | None = None):

        ################################################################################################################

        super().__init__(bind.lib.nyx_blob_prop_new(
            bind.as_bytes(name, allow_none = False),
            bind.as_bytes(label),
            bind.as_bytes(format),
            0,
            None,
            False,
        ))

        ################################################################################################################

        self.value = value

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
@utils.nyx_property(
    'perm',
    '@perm',
    getter = enums.nyx_perm,
    setter = enums.nyx_perm_str,
)
class NyxBlobVector(json.json_dict.NyxDict):

    ####################################################################################################################

    def __init__(self, device: str, name: str, state: enums.NyxState | int | str, perm: enums.NyxPerm | int | str, props: typing.Iterable[NyxBlobProp], **opts: typing.Any):

        ################################################################################################################

        props = tuple(props)

        opts_p = bind.as_opts(opts)

        ################################################################################################################

        super().__init__(bind.lib.nyx_blob_vector_new(
            bind.as_bytes(device, allow_none = False),
            bind.as_bytes(name, allow_none = False),
            enums.nyx_state(state),
            enums.nyx_perm(perm),
            ctypes.c_void_p(),
            opts_p,
        ))

        ################################################################################################################

        children: json.NyxList = self['children']

        for prop in props:

            if not isinstance(prop, NyxBlobProp):

                raise TypeError(f'Expected NyxBlobProp')

            children.push(prop)

        ################################################################################################################

        self._props = props

########################################################################################################################

__all__ = [name for name in globals() if name.lower().startswith('nyx')]

########################################################################################################################
