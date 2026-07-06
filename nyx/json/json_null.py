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

    def __init__(self, ptr = None):
        """!
        @brief Allocates a new JSON null object or wraps one.

        @param ptr Optional JSON null object pointer.
        """

        if ptr is None:

            ptr = bind.lib.nyx_null_new()

        elif bind.lib.nyx_object_get_type(ptr) != bind.NyxObjectType.NULL:

            raise TypeError('Not a pointer to a Nyx null object')

        super().__init__(ptr)

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

        @return `True` if the value was modified, `False` otherwise.
        """

        return False

########################################################################################################################

__all__ = ['NyxNull']

########################################################################################################################
