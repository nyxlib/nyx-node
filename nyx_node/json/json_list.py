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

    ####################################################################################################################

    def __init__(self, ptr = None):

        if ptr is None:

            ptr = bind.lib.nyx_list_new()

        elif bind.lib.nyx_object_get_type(ptr) != 0x65656505:

            raise TypeError('Not a pointer to a Nyx list object')

        super().__init__(ptr)

    ####################################################################################################################

    def clear(self) -> None:

        bind.lib.nyx_list_clear(self.ptr)

    ####################################################################################################################

    def delete(self, idx: int) -> None:

        bind.lib.nyx_list_del(self.ptr, idx)

    ####################################################################################################################

    def get(self, idx: int) -> NyxNull | NyxBoolean | NyxNumber | NyxString | NyxDict | NyxList:

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

            if object_type == 0x65656500: # NYX_TYPE_NULL
                from .json_null import NyxNull
                return NyxNull(ptr)

            if object_type == 0x65656501: # NYX_TYPE_BOOLEAN
                from .json_boolean import NyxBoolean
                return NyxBoolean(ptr)

            if object_type == 0x65656502: # NYX_TYPE_NUMBER
                from .json_number import NyxNumber
                return NyxNumber(ptr)

            if object_type == 0x65656503: # NYX_TYPE_STRING
                from .json_string import NyxString
                return NyxString(ptr)

            if object_type == 0x65656504: # NYX_TYPE_DICT
                from .json_dict import NyxDict
                return NyxDict(ptr)

            if object_type == 0x65656505: # NYX_TYPE_LIST
                #### .json_list import NyxList
                return NyxList(ptr)

            ############################################################################################################

            raise TypeError(f'internal error, unknown Nyx object type `{object_type}`')

            ############################################################################################################

        except BaseException:

            bind.lib.nyx_object_unref(ptr)

            raise

    ####################################################################################################################

    def set(self, idx: int, value: obj.NyxObject) -> bool:

        if not isinstance(value, obj.NyxObject):

            raise TypeError('value must be a nyx_object_t')

        return bool(bind.lib.nyx_list_set(self.ptr, idx, value.ptr))

    ####################################################################################################################

    def push(self, value: obj.NyxObject) -> bool:

        if not isinstance(value, obj.NyxObject):

            raise TypeError('value must be a nyx_object_t')

        return bool(bind.lib.nyx_list_set(self.ptr, -1, value.ptr))

    ####################################################################################################################

    def size(self) -> int:

        return int(bind.lib.nyx_list_size(self.ptr))

########################################################################################################################

__all__ = ['NyxList']

########################################################################################################################
