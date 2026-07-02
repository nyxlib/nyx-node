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
    """Nyx Stream property."""

    ####################################################################################################################

    def __init__(self, name: str, label: str | None = None):
        """Allocates a new Nyx Stream property."""

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
    """Nyx Stream vector."""

    ####################################################################################################################

    def __init__(self, device: str, name: str, state: enums.NyxState | int | str, props: typing.Iterable[NyxStreamProp], **opts: typing.Any):
        """Allocates a new Nyx Stream vector."""

        ################################################################################################################

        super().__init__(bind.lib.nyx_stream_vector_new(
            bind.as_bytes(device, allow_none = False),
            bind.as_bytes(name, allow_none = False),
            enums.nyx_state(state),
            bind.nyx_dict_p(),
            bind.as_opts(opts),
        ))

        ################################################################################################################

        # noinspection PyTypeChecker
        children: json.NyxList = self['children']

        for prop in props:

            if not isinstance(prop, NyxStreamProp):

                raise TypeError(f'Expected NyxStreamProp')

            children.push(prop)

    ####################################################################################################################

    def stream_pub(self, field_values: typing.Sequence[bytes]) -> bool:
        """Publishes an entry to a stream if Nyx Stream is enabled."""

        ################################################################################################################

        field_values = [bind.as_bytes(value, allow_none = False) for value in field_values]

        ################################################################################################################

        cast = lambda value: ctypes.cast(ctypes.c_char_p(value), bind.c_void_p)

        ################################################################################################################

        n_fields = len(field_values)

        field_sizes = (bind.c_size_t * n_fields)(
            *(len(value) for value in field_values),
        )

        field_buffs = (bind.c_void_p * n_fields)(
            *(cast(value) for value in field_values),
        )

        ################################################################################################################

        return bool(bind.lib.nyx_stream_pub(
            self.ptr,
            n_fields,
            field_sizes,
            field_buffs,
        ))

########################################################################################################################

__all__ = [name for name in globals() if name.lower().startswith('nyx')]

########################################################################################################################
