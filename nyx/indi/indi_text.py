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

## @defgroup TEXT_MESSAGE_PY Nyx Text Message
#  @brief Nyx / INDI Text Message.

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
    """!
    @ingroup TEXT_MESSAGE_PY
    @brief INDI / Nyx text property.
    """

    ####################################################################################################################

    def __init__(self, name: str, label: str | None = None, value: str | None = None):
        """!
        @brief Allocates a new INDI / Nyx text property.

        @param name Property name.
        @param label Property label.
        @param value Initial text value.
        """

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
    kind = enums.NyxState,
    getter = enums.NyxState.to_int,
    setter = enums.NyxState.to_str,
)
@utils.nyx_property(
    'perm',
    '@perm',
    kind = enums.NyxPerm,
    getter = enums.NyxPerm.to_int,
    setter = enums.NyxPerm.to_str,
)
class NyxTextVector(json.json_dict.NyxDict):
    """!
    @ingroup TEXT_MESSAGE_PY
    @brief INDI / Nyx text vector.
    """

    ####################################################################################################################

    def __init__(self, device: str, name: str, state: enums.NyxState | int | str, perm: enums.NyxPerm | int | str, props: typing.Iterable[NyxTextProp], **opts: typing.Any):
        """!
        @brief Allocates a new INDI / Nyx text vector.

        @param device Device name.
        @param name Vector name.
        @param state Vector state.
        @param perm Vector permissions.
        @param props Properties.
        @param opts Options (group, label, hints, timeout, message).
        """

        ################################################################################################################

        super().__init__(bind.lib.nyx_text_vector_new(
            bind.as_bytes(device, allow_none = False),
            bind.as_bytes(name, allow_none = False),
            enums.NyxState.to_int(state),
            enums.NyxPerm.to_int(perm),
            bind.nyx_dict_p(),
            bind.as_opts(opts),
        ))

        ################################################################################################################

        # noinspection PyTypeChecker
        children: json.NyxList = self['children']

        for prop in props:

            if not isinstance(prop, NyxTextProp):

                raise TypeError('Expected NyxTextProp')

            children.push(prop)

    ####################################################################################################################

    @obj.nyx_callback(bind.nyx_callback_vector_t)
    def _nyx_callback_method(self, _vector: json.json_dict.NyxDict, modified: bool) -> None:

        self._dispatch_callbacks(bool(modified))

########################################################################################################################

__all__ = [name for name in globals() if name.lower().startswith('nyx')]

########################################################################################################################
