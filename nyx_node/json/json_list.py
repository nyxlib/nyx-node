########################################################################################################################
# NyxNode
# Author: Jérôme ODIER <jerome.odier@lpsc.in2p3.fr>
# SPDX-License-Identifier: GPL-3.0+
########################################################################################################################

from __future__ import annotations

########################################################################################################################

import typing

########################################################################################################################

from .. import bind
from .. import obj

if typing.TYPE_CHECKING:

    from .json_null import NyxNull
    from .json_boolean import NyxBoolean
    from .json_number import NyxNumber
    from .json_string import NyxString
    from .json_dict import NyxDict

########################################################################################################################

class NyxList(obj.NyxObject):
    """JSON list object."""

    ####################################################################################################################

    def __init__(self, ptr = None):
        """Allocates a new JSON list object or wraps one."""

        if ptr is None:

            ptr = bind.lib.nyx_list_new()

        elif bind.lib.nyx_object_get_type(ptr) != bind.NyxObjectType.LIST:

            raise TypeError('Not a pointer to a Nyx list object')

        super().__init__(ptr)

    ####################################################################################################################

    def clear(self) -> None:
        """Clears the content of this JSON list object."""

        bind.lib.nyx_list_clear(self.ptr)

    ####################################################################################################################

    def __delitem__(self, idx: int) -> None:
        """Deletes the entry at the provided index."""

        bind.lib.nyx_list_del(self.ptr, idx)

    ####################################################################################################################

    def __getitem__(self, idx: int) -> NyxNull | NyxBoolean | NyxNumber | NyxString | NyxDict | NyxList:
        """Gets the JSON object at the provided index."""

        ################################################################################################################

        ptr = bind.lib.nyx_list_get(self.ptr, idx)

        if not ptr:

            raise IndexError(idx)

        ################################################################################################################

        bind.lib.nyx_object_ref(ptr)

        ################################################################################################################

        try:

            ############################################################################################################

            object_type = bind.lib.nyx_object_get_type(ptr)

            ############################################################################################################

            if object_type == bind.NyxObjectType.NULL:
                from .json_null import NyxNull
                return NyxNull(ptr)

            if object_type == bind.NyxObjectType.BOOLEAN:
                from .json_boolean import NyxBoolean
                return NyxBoolean(ptr)

            if object_type == bind.NyxObjectType.NUMBER:
                from .json_number import NyxNumber
                return NyxNumber(ptr)

            if object_type == bind.NyxObjectType.STRING:
                from .json_string import NyxString
                return NyxString(ptr)

            if object_type == bind.NyxObjectType.DICT:
                from .json_dict import NyxDict
                return NyxDict(ptr)

            if object_type == bind.NyxObjectType.LIST:
                #### .json_list import NyxList
                return NyxList(ptr)

            ############################################################################################################

            raise TypeError(f'internal error, unknown Nyx object type `{object_type}`')

            ############################################################################################################

        except BaseException:

            bind.lib.nyx_object_unref(ptr)

            raise

    ####################################################################################################################

    def __setitem__(self, idx: int, value: obj.NyxObject) -> bool:
        """Sets a JSON object at the provided index."""

        if not isinstance(value, obj.NyxObject):

            raise TypeError('value must be a Nyx object')

        return bool(bind.lib.nyx_list_set(self.ptr, idx, value.ptr))

    ####################################################################################################################

    def push(self, value: obj.NyxObject) -> bool:
        """Pushes a JSON object in this JSON list object."""

        return self.__setitem__(-1, value)

    ####################################################################################################################

    def __len__(self) -> int:
        """Returns the number of items."""

        return int(bind.lib.nyx_list_size(self.ptr))

########################################################################################################################

__all__ = ['NyxList']

########################################################################################################################
