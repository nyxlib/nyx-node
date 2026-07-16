# -*- coding: utf-8 -*-
########################################################################################################################
# NyxNode
# Author: Jérôme ODIER <jerome.odier@lpsc.in2p3.fr>
# SPDX-License-Identifier: GPL-3.0+
########################################################################################################################

import typing

########################################################################################################################

from .. import bind
from .. import obj

########################################################################################################################

class NyxNull(obj.NyxObject):
    """!
    @brief JSON null object.
    """

    ####################################################################################################################

    def __init__(self, value: typing.Any, ptr: int | None = None):
        """!
        @brief Allocates a new JSON null object or wraps one.

        @param value Optional default value.
        @param ptr Optional JSON null object pointer.
        """

        ################################################################################################################

        if ptr is None:

            ptr = bind.lib.nyx_null_new()

        elif bind.lib.nyx_object_get_type(ptr) != bind.NyxObjectType.NULL:

            raise TypeError('Not a pointer to a Nyx null object')

        ################################################################################################################

        super().__init__(ptr)

        ################################################################################################################

        if isinstance(value, type(None)):

            raise TypeError('Cannot set a value on a JSON null object')

    ####################################################################################################################

    @property
    def value(self) -> None:
        """!
        @brief Gets the value of this JSON null object.

        @return The current value (`None`).
        """

        return None

    ####################################################################################################################

    # noinspection PyUnusedLocal
    @value.setter
    def value(self, value: typing.Any) -> bool:
        """!
        @brief Sets the value of this JSON null object.

        @return @c True if the value was modified, @c False otherwise.
        """

        return False

########################################################################################################################

__all__ = ['NyxNull']

########################################################################################################################
