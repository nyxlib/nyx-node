# -*- coding: utf-8 -*-
########################################################################################################################
# NyxNode
# Author: Jérôme ODIER <jerome.odier@lpsc.in2p3.fr>
# SPDX-License-Identifier: GPL-3.0+
########################################################################################################################

from .. import bind
from .. import obj

########################################################################################################################

class NyxNumber(obj.NyxObject):
    """!
    @brief JSON number object.
    """

    ####################################################################################################################

    def __init__(self, value: int | float | None = None, ptr: int | None = None):
        """!
        @brief Allocates a new JSON number object or wraps one.

        @param value Optional default value.
        @param ptr Optional JSON number object pointer.
        """

        ################################################################################################################

        if ptr is None:

            ptr = bind.lib.nyx_number_new()

        elif bind.lib.nyx_object_get_type(ptr) != bind.NyxObjectType.NUMBER:

            raise TypeError('Not a pointer to a Nyx number object')

        ################################################################################################################

        super().__init__(ptr)

        ################################################################################################################

        if isinstance(value, (int, float)):

            self.value = float(value)

    ####################################################################################################################

    @property
    def value(self) -> float:
        """!
        @brief Gets the value of this JSON number object.

        @return The current value.
        """

        return float(bind.lib.nyx_number_get(self.ptr))

    ####################################################################################################################

    @value.setter
    def value(self, value: float) -> bool:
        """!
        @brief Sets the value of this JSON number object.

        @return @c True if the value was modified, @c False otherwise.
        """

        return bool(bind.lib.nyx_number_set(self.ptr, float(value)))

########################################################################################################################

__all__ = ['NyxNumber']

########################################################################################################################
