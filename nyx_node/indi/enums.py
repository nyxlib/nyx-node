# -*- coding: utf-8 -*-
########################################################################################################################
# NyxNode
# Author: Jérôme ODIER <jerome.odier@lpsc.in2p3.fr>
# SPDX-License-Identifier: GPL-3.0+
########################################################################################################################

from __future__ import annotations

########################################################################################################################

import enum
import typing

########################################################################################################################

class NyxState(enum.IntEnum):
    """! @brief Vector state hint."""

    IDLE = 400
    OK = 401
    BUSY = 402
    ALERT = 403

    ####################################################################################################################

    @classmethod
    def nyx_state_int(cls, value: NyxState | int | str) -> int:
        """! @brief Converts a NyxState value as an integer.

        @param value The value to convert.
        @return The NyxState value as an integer.
        """

        return _nyx_enum_int(value, NyxState, _STATE_FROM_STR, 'state')

    ####################################################################################################################

    @classmethod
    def nyx_state_str(cls, value: NyxState | int | str) -> str:
        """! @brief Converts a NyxState value as a string.

        @param value The value to convert.
        @return The NyxState value as a string.
        """

        return _nyx_enum_str(value, NyxState, _STATE_TO_STR, _STATE_FROM_STR, 'state')

########################################################################################################################

_STATE_TO_STR = {
    NyxState.IDLE: 'Idle',
    NyxState.OK: 'Ok',
    NyxState.BUSY: 'Busy',
    NyxState.ALERT: 'Alert',
}

_STATE_FROM_STR = {value: key for key, value in _STATE_TO_STR.items()}

########################################################################################################################

class NyxPerm(enum.IntEnum):
    """! @brief Vector permission hint."""

    RO = 500
    WO = 501
    RW = 502

    ####################################################################################################################

    @classmethod
    def nyx_perm_int(cls, value: NyxPerm | int | str) -> int:
        """! @brief Converts a NyxPerm value as an integer.

        @param value The value to convert.
        @return The NyxPerm value as an integer.
        """

        return _nyx_enum_int(value, NyxPerm, _PERM_FROM_STR, 'permission')

    ####################################################################################################################

    @classmethod
    def nyx_perm_str(cls, value: NyxPerm | int | str) -> str:
        """! @brief Converts a NyxPerm value as a string.

        @param value The value to convert.
        @return The NyxPerm value as a string.
        """

        return _nyx_enum_str(value, NyxPerm, _PERM_TO_STR, _PERM_FROM_STR, 'permission')

########################################################################################################################

_PERM_TO_STR = {
    NyxPerm.RO: 'ro',
    NyxPerm.WO: 'wo',
    NyxPerm.RW: 'rw',
}

_PERM_FROM_STR = {value: key for key, value in _PERM_TO_STR.items()}

########################################################################################################################

class NyxRule(enum.IntEnum):
    """! @brief Switch vector rule hint."""

    ONE_OF_MANY = 600
    AT_MOST_ONE = 601
    ANY_OF_MANY = 602

    ####################################################################################################################

    @classmethod
    def nyx_rule_int(cls, value: NyxRule | int | str) -> int:
        """! @brief Converts a NyxRule value as an integer.

        @param value The value to convert.
        @return The NyxRule value as an integer.
        """

        return _nyx_enum_int(value, NyxRule, _RULE_FROM_STR, 'rule')

    ####################################################################################################################

    @classmethod
    def nyx_rule_str(cls, value: NyxRule | int | str) -> str:
        """! @brief Converts a NyxRule value as a string.

        @param value The value to convert.
        @return The NyxRule value as a string.
        """

        return _nyx_enum_str(value, NyxRule, _RULE_TO_STR, _RULE_FROM_STR, 'rule')

########################################################################################################################

_RULE_TO_STR = {
    NyxRule.ONE_OF_MANY: 'OneOfMany',
    NyxRule.AT_MOST_ONE: 'AtMostOne',
    NyxRule.ANY_OF_MANY: 'AnyOfMany',
}

_RULE_FROM_STR = {value: key for key, value in _RULE_TO_STR.items()}

########################################################################################################################

class NyxOnOff(enum.IntEnum):
    """! @brief Switch state."""

    ON = 700
    OFF = 701

    ####################################################################################################################

    @classmethod
    def nyx_onoff_int(cls, value: NyxOnOff | int | str | bool) -> int:
        """! @brief Converts a NyxOnOff value as an integer.

        @param value The value to convert.
        @return The NyxOnOff value as an integer.
        """

        if isinstance(value, bool):

            return int(NyxOnOff.ON if value else NyxOnOff.OFF)

        return _nyx_enum_int(value, NyxOnOff, _ONOFF_FROM_STR, 'onoff')

    ####################################################################################################################

    @classmethod
    def nyx_onoff_str(cls, value: NyxOnOff | int | str | bool) -> str:
        """! @brief Converts a NyxOnOff value as a string.

        @param value The value to convert.
        @return The NyxOnOff value as a string.
        """

        if isinstance(value, bool):

            return 'On' if value else 'Off'

        return _nyx_enum_str(value, NyxOnOff, _ONOFF_TO_STR, _ONOFF_FROM_STR, 'onoff')

########################################################################################################################

_ONOFF_TO_STR = {
    NyxOnOff.ON: 'On',
    NyxOnOff.OFF: 'Off',
}

_ONOFF_FROM_STR = {value: key for key, value in _ONOFF_TO_STR.items()}

########################################################################################################################

_NyxEnum = typing.TypeVar('_NyxEnum', bound = enum.IntEnum)

########################################################################################################################

def _nyx_enum_int(value: _NyxEnum | int | str, enum_type: type[_NyxEnum], from_str: typing.Mapping[str, _NyxEnum], name: str) -> int:

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

    return to_str[enum_type(_nyx_enum_int(value, enum_type, from_str, name))]

########################################################################################################################

__all__ = [name for name in globals() if name.lower().startswith('nyx')]

########################################################################################################################
