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

## @defgroup SWITCH_MESSAGE_PY Nyx Switch Message
#  @brief Nyx / INDI Switch Message.

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
    getter = enums.NyxOnOff.to_int,
    setter = enums.NyxOnOff.to_str,
)
class NyxSwitchProp(json.json_dict.NyxDict):
    """!
    @ingroup SWITCH_MESSAGE_PY
    @brief INDI / Nyx switch property.
    """

    ####################################################################################################################

    def __init__(self, name: str, label: str | None = None, value: enums.NyxOnOff | int | str | bool = enums.NyxOnOff.OFF):
        """!
        @brief Allocates a new INDI / Nyx switch property.

        @param name Property name.
        @param label Property label.
        @param value Initial value.
        """

        super().__init__(bind.lib.nyx_switch_prop_new(
            bind.as_bytes(name, allow_none = False),
            bind.as_bytes(label, allow_none = True),
            enums.NyxOnOff.to_int(value),
        ))

    ####################################################################################################################

    @obj.nyx_callback(bind.nyx_callback_int_t)
    def _nyx_callback_method(self, _vector, _prop, new_value, old_value):

        return all(self._dispatch_callbacks(
            enums.NyxOnOff.to_int(new_value),
            enums.NyxOnOff.to_int(old_value),
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
    getter = enums.NyxState.to_int,
    setter = enums.NyxState.to_str,
)
@utils.nyx_property(
    'perm',
    '@perm',
    getter = enums.NyxPerm.to_int,
    setter = enums.NyxPerm.to_str,
)
@utils.nyx_property(
    'rule',
    '@rule',
    getter = enums.NyxRule.nyx_rule_int,
    setter = enums.NyxRule.nyx_rule_str,
)
class NyxSwitchVector(json.json_dict.NyxDict):
    """!
    @ingroup SWITCH_MESSAGE_PY
    @brief INDI / Nyx switch vector.
    """

    ####################################################################################################################

    def __init__(self, device: str, name: str, state: enums.NyxState | int | str, perm: enums.NyxPerm | int | str, rule: enums.NyxRule | int | str, props: typing.Iterable[NyxSwitchProp], **opts: typing.Any):
        """!
        @brief Allocates a new INDI / Nyx switch vector.

        @param device Device name.
        @param name Vector name.
        @param state Vector state.
        @param perm Vector permissions.
        @param rule Vector rules.
        @param props Properties.
        @param opts Options (group, label, hints, timeout, message).
        """

        ################################################################################################################

        super().__init__(bind.lib.nyx_switch_vector_new(
            bind.as_bytes(device, allow_none = False),
            bind.as_bytes(name, allow_none = False),
            enums.NyxState.to_int(state),
            enums.NyxPerm.to_int(perm),
            enums.NyxRule.nyx_rule_int(rule),
            bind.nyx_dict_p(),
            bind.as_opts(opts),
        ))

        ################################################################################################################

        # noinspection PyTypeChecker
        children: json.NyxList = self['children']

        for prop in props:

            if not isinstance(prop, NyxSwitchProp):

                raise TypeError('Expected NyxSwitchProp')

            children.push(prop)

    ####################################################################################################################

    @obj.nyx_callback(bind.nyx_callback_vector_t)
    def _nyx_callback_method(self, _vector: json.json_dict.NyxDict, modified: bool) -> None:

        self._dispatch_callbacks(bool(modified))

########################################################################################################################

__all__ = [name for name in globals() if name.lower().startswith('nyx')]

########################################################################################################################
