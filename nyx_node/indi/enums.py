# -*- coding: utf-8 -*-
########################################################################################################################
# NyxNode
# Author: Jérôme ODIER <jerome.odier@lpsc.in2p3.fr>
# SPDX-License-Identifier: GPL-3.0+
########################################################################################################################

import enum
import typing

########################################################################################################################
# TYPES                                                                                                                #
########################################################################################################################

class NyxState(enum.IntEnum):

    NYX_STATE_IDLE = 400
    NYX_STATE_OK = 401
    NYX_STATE_BUSY = 402
    NYX_STATE_ALERT = 403

########################################################################################################################

class NyxPerm(enum.IntEnum):

    NYX_PERM_RO = 500
    NYX_PERM_WO = 501
    NYX_PERM_RW = 502

########################################################################################################################

class NyxRule(enum.IntEnum):

    NYX_RULE_ONE_OF_MANY = 600
    NYX_RULE_AT_MOST_ONE = 601
    NYX_RULE_ANY_OF_MANY = 602

########################################################################################################################

class NyxOnOff(enum.IntEnum):

    NYX_ONOFF_ON = 700
    NYX_ONOFF_OFF = 701

########################################################################################################################
# PRIVATE HELPERS                                                                                                      #
########################################################################################################################

_NyxEnum = typing.TypeVar('_NyxEnum', bound = enum.IntEnum)

########################################################################################################################

def _nyx_enum(value: _NyxEnum | int | str, enum_type: type[_NyxEnum], from_str: typing.Mapping[str, _NyxEnum], name: str) -> int:

    ####################################################################################################################

    if isinstance(value, enum_type):

        return int(value)

    ####################################################################################################################

    if type(value) is int:

        try:
            return int(enum_type(value))
        except ValueError:
            raise ValueError(f'Invalid Nyx {name}: {value!r}') from None

    ####################################################################################################################

    if isinstance(value, str):

        try:
            return int(from_str[value])
        except KeyError:
            raise ValueError(f'Invalid Nyx {name}: {value!r}') from None

    ####################################################################################################################

    raise TypeError(f'Expected {enum_type.__name__}, int or str, got {type(value).__name__}')

########################################################################################################################

def _nyx_enum_str(value: _NyxEnum | int | str, enum_type: type[_NyxEnum], to_str: typing.Mapping[_NyxEnum, str], from_str: typing.Mapping[str, _NyxEnum], name: str) -> str:

    return to_str[enum_type(_nyx_enum(value, enum_type, from_str, name))]

########################################################################################################################

_STATE_TO_STR = {
    NyxState.NYX_STATE_IDLE: 'Idle',
    NyxState.NYX_STATE_OK: 'Ok',
    NyxState.NYX_STATE_BUSY: 'Busy',
    NyxState.NYX_STATE_ALERT: 'Alert',
}

_STATE_FROM_STR = {value: key for key, value in _STATE_TO_STR.items()}

########################################################################################################################

_PERM_TO_STR = {
    NyxPerm.NYX_PERM_RO: 'ro',
    NyxPerm.NYX_PERM_WO: 'wo',
    NyxPerm.NYX_PERM_RW: 'rw',
}

_PERM_FROM_STR = {value: key for key, value in _PERM_TO_STR.items()}

########################################################################################################################

_RULE_TO_STR = {
    NyxRule.NYX_RULE_ONE_OF_MANY: 'OneOfMany',
    NyxRule.NYX_RULE_AT_MOST_ONE: 'AtMostOne',
    NyxRule.NYX_RULE_ANY_OF_MANY: 'AnyOfMany',
}

_RULE_FROM_STR = {value: key for key, value in _RULE_TO_STR.items()}

########################################################################################################################

_ONOFF_TO_STR = {
    NyxOnOff.NYX_ONOFF_ON: 'On',
    NyxOnOff.NYX_ONOFF_OFF: 'Off',
}

_ONOFF_FROM_STR = {value: key for key, value in _ONOFF_TO_STR.items()}

########################################################################################################################
# PUBLIC                                                                                                               #
########################################################################################################################

def nyx_state(value: NyxState | int | str) -> int:

    return _nyx_enum(value, NyxState, _STATE_FROM_STR, 'state')

########################################################################################################################

def nyx_state_str(value: NyxState | int | str) -> str:

    return _nyx_enum_str(value, NyxState, _STATE_TO_STR, _STATE_FROM_STR, 'state')

########################################################################################################################

def nyx_perm(value: NyxPerm | int | str) -> int:

    return _nyx_enum(value, NyxPerm, _PERM_FROM_STR, 'permission')

########################################################################################################################

def nyx_perm_str(value: NyxPerm | int | str) -> str:

    return _nyx_enum_str(value, NyxPerm, _PERM_TO_STR, _PERM_FROM_STR, 'permission')

########################################################################################################################

def nyx_rule(value: NyxRule | int | str) -> int:

    return _nyx_enum(value, NyxRule, _RULE_FROM_STR, 'rule')

########################################################################################################################

def nyx_rule_str(value: NyxRule | int | str) -> str:

    return _nyx_enum_str(value, NyxRule, _RULE_TO_STR, _RULE_FROM_STR, 'rule')

########################################################################################################################

def nyx_onoff(value: NyxOnOff | int | str | bool) -> int:

    if isinstance(value, bool):

        return int(NyxOnOff.NYX_ONOFF_ON if value else NyxOnOff.NYX_ONOFF_OFF)

    return _nyx_enum(value, NyxOnOff, _ONOFF_FROM_STR, 'OnOff value')

########################################################################################################################

def nyx_onoff_str(value: NyxOnOff | int | str | bool) -> str:

    return _ONOFF_TO_STR[NyxOnOff(nyx_onoff(value))]

########################################################################################################################

__all__ = [name for name in globals() if name.lower().startswith('nyx')]

########################################################################################################################
