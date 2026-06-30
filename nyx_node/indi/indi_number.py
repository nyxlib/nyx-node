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
class NyxNumberProp(json.json_dict.NyxDict):

    pass

########################################################################################################################

class NyxNumberIntProp(NyxNumberProp):

    ####################################################################################################################

    def __init__(self, name: str, label: str | None, fmt: str, min: int, max: int, step: int, value: int):

        super().__init__(bind.lib.nyx_number_prop_new_int(
            bind.as_bytes(name, allow_none = False),
            bind.as_bytes(label, allow_none = False),
            bind.as_bytes(fmt, allow_none = False),
            min,
            max,
            step,
            value,
        ))

    ####################################################################################################################

    @property
    def value(self) -> int:

        return bind.lib.nyx_number_prop_get_int(self.ptr)

    ####################################################################################################################

    @value.setter
    def value(self, value: int) -> None:

        bind.lib.nyx_number_prop_set_int(
            self.ptr,
            value,
        )

########################################################################################################################

class NyxNumberUIntProp(NyxNumberProp):

    ####################################################################################################################

    def __init__(self, name: str, label: str | None, fmt: str, min: int, max: int, step: int, value: int):

        super().__init__(bind.lib.nyx_number_prop_new_uint(
            bind.as_bytes(name, allow_none = False),
            bind.as_bytes(label, allow_none = True),
            bind.as_bytes(fmt, allow_none = False),
            min,
            max,
            step,
            value,
        ))

    ####################################################################################################################

    @property
    def value(self) -> int:

        return bind.lib.nyx_number_prop_get_uint(self.ptr)

    ####################################################################################################################

    @value.setter
    def value(self, value: int) -> None:

        bind.lib.nyx_number_prop_set_uint(
            self.ptr,
            value,
        )

########################################################################################################################

class NyxNumberLongProp(NyxNumberProp):

    ####################################################################################################################

    def __init__(self, name: str, label: str | None, fmt: str, min: int, max: int, step: int, value: int):

        super().__init__(bind.lib.nyx_number_prop_new_long(
            bind.as_bytes(name, allow_none = False),
            bind.as_bytes(label, allow_none = True),
            bind.as_bytes(fmt, allow_none = False),
            min,
            max,
            step,
            value,
        ))

    ####################################################################################################################

    @property
    def value(self) -> int:

        return bind.lib.nyx_number_prop_get_long(self.ptr)

    ####################################################################################################################

    @value.setter
    def value(self, value: int) -> None:

        bind.lib.nyx_number_prop_set_long(
            self.ptr,
            value,
        )

########################################################################################################################

class NyxNumberULongProp(NyxNumberProp):

    ####################################################################################################################

    def __init__(self, name: str, label: str | None, fmt: str, min: int, max: int, step: int, value: int):

        super().__init__(bind.lib.nyx_number_prop_new_ulong(
            bind.as_bytes(name, allow_none = False),
            bind.as_bytes(label, allow_none = True),
            bind.as_bytes(fmt, allow_none = False),
            min,
            max,
            step,
            value,
        ))

    ####################################################################################################################

    @property
    def value(self) -> int:

        return bind.lib.nyx_number_prop_get_ulong(self.ptr)

    ####################################################################################################################

    @value.setter
    def value(self, value: int) -> None:

        bind.lib.nyx_number_prop_set_ulong(
            self.ptr,
            value,
        )

########################################################################################################################

class NyxNumberDoubleProp(NyxNumberProp):

    ####################################################################################################################

    def __init__(self, name: str, label: str | None, fmt: str, min: float, max: float, step: float, value: float):

        super().__init__(bind.lib.nyx_number_prop_new_double(
            bind.as_bytes(name, allow_none = False),
            bind.as_bytes(label, allow_none = True),
            bind.as_bytes(fmt, allow_none = False),
            min,
            max,
            step,
            value,
        ))

    ####################################################################################################################

    @property
    def value(self) -> float:

        return bind.lib.nyx_number_prop_get_double(self.ptr)

    ####################################################################################################################

    @value.setter
    def value(self, value: float) -> None:

        bind.lib.nyx_number_prop_set_double(
            self.ptr,
            value,
        )

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
class NyxNumberVector(json.json_dict.NyxDict):

    ####################################################################################################################

    def __init__(self, device: str, name: str, state: enums.NyxState | int | str, perm: enums.NyxPerm | int | str, props: typing.Iterable[NyxNumberProp], **opts: typing.Any):

        ################################################################################################################

        super().__init__(bind.lib.nyx_number_vector_new(
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

            if not isinstance(prop, NyxNumberProp):

                raise TypeError('Expected NyxNumberProp')

            children.push(prop)

########################################################################################################################

__all__ = [name for name in globals() if name.lower().startswith('nyx')]

########################################################################################################################
