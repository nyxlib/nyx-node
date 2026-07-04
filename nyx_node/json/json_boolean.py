# -*- coding: utf-8 -*-
########################################################################################################################
# NyxNode
# Author: Jérôme ODIER <jerome.odier@lpsc.in2p3.fr>
# SPDX-License-Identifier: GPL-3.0+
########################################################################################################################

from .. import bind
from .. import obj

########################################################################################################################

class NyxBoolean(obj.NyxObject):
    """! @brief JSON boolean object."""

    ####################################################################################################################

    def __init__(self, ptr = None):
        """! @brief Allocates a new JSON boolean object or wraps one.

        @param ptr Optional JSON boolean object pointer.
        """

        if ptr is None:

            ptr = bind.lib.nyx_boolean_new()

        elif bind.lib.nyx_object_get_type(ptr) != bind.NyxObjectType.BOOLEAN:

            raise TypeError('Not a pointer to a Nyx boolean object')

        super().__init__(ptr)

    ####################################################################################################################

    @property
    def value(self) -> bool:
        """! @brief Gets the value of this JSON boolean object.

        @return The value.
        """

        return bool(bind.lib.nyx_boolean_get(self.ptr))

    ####################################################################################################################

    @value.setter
    def value(self, value: bool) -> bool:

        return bool(bind.lib.nyx_boolean_set(self.ptr, bool(value)))

########################################################################################################################

__all__ = ['NyxBoolean']

########################################################################################################################
