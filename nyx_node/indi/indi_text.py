# -*- coding: utf-8 -*-
########################################################################################################################
# NyxNode
# Author: Jérôme ODIER <jerome.odier@lpsc.in2p3.fr>
# SPDX-License-Identifier: GPL-3.0+
########################################################################################################################

import typing

########################################################################################################################

from .. import obj
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
    'value',
    '$',
)
class NyxTextProp(json.json_dict.NyxDict):

    ####################################################################################################################

    def __init__(self, name: str, label: str | None = None, value: str | None = None):

        super().__init__(bind.lib.nyx_text_prop_new(
            bind.as_bytes(name, allow_none = False),
            bind.as_bytes(label, allow_none = True),
            bind.as_bytes(value, allow_none = True),
        ))

    ####################################################################################################################

    @obj.nyx_callback(bind.nyx_callback_str_t)
    def _nyx_callback_method(self, _vector, _prop, new_value, old_value):

        return all(self._dispatch_callbacks(
            new_value.decode('utf-8') if new_value is not None else None,
            old_value.decode('utf-8') if old_value is not None else None,
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
@utils.nyx_property(
    'perm',
    '@perm',
    getter = enums.nyx_perm,
    setter = enums.nyx_perm_str,
)
class NyxTextVector(json.json_dict.NyxDict):

    ####################################################################################################################

    def __init__(self, device: str, name: str, state: enums.NyxState | int | str, perm: enums.NyxPerm | int | str, props: typing.Iterable[NyxTextProp], **opts: typing.Any):

        ################################################################################################################

        super().__init__(bind.lib.nyx_text_vector_new(
            bind.as_bytes(device, allow_none = False),
            bind.as_bytes(name, allow_none = False),
            enums.nyx_state(state),
            enums.nyx_perm(perm),
            bind.nyx_dict_p(),
            bind.as_opts(opts),
        ))

        ################################################################################################################

        # noinspection PyTypeChecker
        children: json.NyxList = self['children']

        for prop in props:

            if not isinstance(prop, NyxTextProp):

                raise TypeError(f'Expected NyxTextProp')

            children.push(prop)


    ####################################################################################################################

    @obj.nyx_callback(bind.nyx_callback_vector_t)
    def _nyx_callback_method(self, _vector, modified):

        self._dispatch_callbacks(bool(modified))

########################################################################################################################

__all__ = [name for name in globals() if name.lower().startswith('nyx')]

########################################################################################################################
