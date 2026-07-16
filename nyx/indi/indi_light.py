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

## @defgroup LIGHT_MESSAGE_PY Nyx Light Message
#  @brief Nyx / INDI Light Message.

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
    kind = enums.NyxState,
    getter = enums.NyxState.to_int,
    setter = enums.NyxState.to_str,
)
class NyxLightProp(json.json_dict.NyxDict):
    """!
    @ingroup LIGHT_MESSAGE_PY
    @brief INDI / Nyx light property.
    """

    ####################################################################################################################

    def __init__(self, name: str, label: str | None = None, value: enums.NyxState | int | str = enums.NyxState.IDLE):
        """!
        @brief Allocates a new INDI / Nyx light property.

        @param name Property name.
        @param label Property label.
        @param value Initial value.
        """

        super().__init__(bind.lib.nyx_light_prop_new(
            bind.as_bytes(name, allow_none = False),
            bind.as_bytes(label, allow_none = True),
            enums.NyxState.to_int(value),
        ))

    ####################################################################################################################

    @obj.nyx_callback(bind.nyx_callback_int_t)
    def _nyx_callback_func(self, _vector, _prop, new_value, old_value) -> bool:

        return all(self._dispatch_callbacks(
            enums.NyxState.to_int(new_value),
            enums.NyxState.to_int(old_value),
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
    kind = enums.NyxState,
    getter = enums.NyxState.to_int,
    setter = enums.NyxState.to_str,
)
class NyxLightVector(json.json_dict.NyxDict):
    """!
    @ingroup LIGHT_MESSAGE_PY
    @brief INDI / Nyx light vector.
    """

    ####################################################################################################################

    def __init__(self, device: str, name: str, state: enums.NyxState | int | str, props: typing.Iterable[NyxLightProp], **opts: typing.Any):
        """!
        @brief Allocates a new INDI / Nyx light vector.

        @param device Device name.
        @param name Vector name.
        @param state Vector state.
        @param props Properties.
        @param opts Options (group, label, hints, timeout, message).
        """

        ################################################################################################################

        super().__init__(bind.lib.nyx_light_vector_new(
            bind.as_bytes(device, allow_none = False),
            bind.as_bytes(name, allow_none = False),
            enums.NyxState.to_int(state),
            bind.nyx_dict_p(),
            bind.as_opts(opts),
        ))

        ################################################################################################################

        # noinspection PyTypeChecker
        children: json.NyxList = self['children']

        for prop in props:

            if not isinstance(prop, NyxLightProp):

                raise TypeError('Expected NyxLightProp')

            children.append(prop)

    ####################################################################################################################

    @obj.nyx_callback(bind.nyx_callback_vector_t)
    def _nyx_callback_func(self, _vector: json.json_dict.NyxDict, modified: bool) -> None:

        self._dispatch_callbacks(bool(modified))

########################################################################################################################

__all__ = [name for name in globals() if name.lower().startswith('nyx')]

########################################################################################################################
