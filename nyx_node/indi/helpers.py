# -*- coding: utf-8 -*-
########################################################################################################################
# NyxNode
# Author: Jérôme ODIER <jerome.odier@lpsc.in2p3.fr>
# SPDX-License-Identifier: GPL-3.0+
########################################################################################################################

from enum import IntEnum

########################################################################################################################

class NyxState(IntEnum):

    NYX_STATE_IDLE = 400
    NYX_STATE_OK = 401
    NYX_STATE_BUSY = 402
    NYX_STATE_ALERT = 403

########################################################################################################################

class NyxPerm(IntEnum):

    NYX_PERM_RO = 500
    NYX_PERM_WO = 501
    NYX_PERM_RW = 502

########################################################################################################################

class NyxRule(IntEnum):

    NYX_RULE_ONE_OF_MANY = 600
    NYX_RULE_AT_MOST_ONE = 601
    NYX_RULE_ANY_OF_MANY = 602

########################################################################################################################

class NyxOnOff(IntEnum):

    NYX_ONOFF_ON = 700
    NYX_ONOFF_OFF = 701

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

def nyx_state(value: NyxState | int | str) -> int:

    if isinstance(value, str):

        return int(_STATE_FROM_STR[value])

    return int(NyxState(value))

########################################################################################################################

def nyx_state_str(value: NyxState | int | str) -> str:

    return _STATE_TO_STR[NyxState(nyx_state(value))]

########################################################################################################################

def nyx_perm(value: NyxPerm | int | str) -> int:

    if isinstance(value, str):

        return int(_PERM_FROM_STR[value])

    return int(NyxPerm(value))

########################################################################################################################

def nyx_perm_str(value: NyxPerm | int | str) -> str:

    return _PERM_TO_STR[NyxPerm(nyx_perm(value))]

########################################################################################################################

def nyx_rule(value: NyxRule | int | str) -> int:

    if isinstance(value, str):

        return int(_RULE_FROM_STR[value])

    return int(NyxRule(value))

########################################################################################################################

def nyx_rule_str(value: NyxRule | int | str) -> str:

    return _RULE_TO_STR[NyxRule(nyx_rule(value))]

########################################################################################################################

def nyx_onoff(value: NyxOnOff | int | str | bool) -> int:

    if isinstance(value, bool):

        return int(NyxOnOff.NYX_ONOFF_ON if value else NyxOnOff.NYX_ONOFF_OFF)

    if isinstance(value, str):

        return int(_ONOFF_FROM_STR[value])

    return int(NyxOnOff(value))

########################################################################################################################

def nyx_onoff_str(value: NyxOnOff | int | str | bool) -> str:

    return _ONOFF_TO_STR[NyxOnOff(nyx_onoff(value))]

########################################################################################################################

__all__ = [name for name in globals() if name.lower().startswith('nyx')]

########################################################################################################################
