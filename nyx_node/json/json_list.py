########################################################################################################################
# NyxNode
# Author: Jérôme ODIER <jerome.odier@lpsc.in2p3.fr>
# SPDX-License-Identifier: GPL-3.0+
########################################################################################################################

import typing

########################################################################################################################

from .. import ffi
from .. import obj

if typing.TYPE_CHECKING:

    from .json_null import NyxNull
    from .json_boolean import NyxBoolean
    from .json_number import NyxNumber
    from .json_string import NyxString
    from .json_dict import NyxDict

########################################################################################################################

class nyx_object_list_t(obj.NyxObject):

    ####################################################################################################################

    def __init__(self, ptr = None):

        if ptr is None:

            ptr = ffi.lib.nyx_list_new()

        super().__init__(ptr)

    ####################################################################################################################

    def clear(self) -> None:

        ffi.lib.nyx_list_clear(self.ptr)

    ####################################################################################################################

    def delete(self, idx: int) -> None:

        ffi.lib.nyx_list_del(self.ptr, idx)

    ####################################################################################################################

    def get(self, idx: int) -> NyxNull | NyxBoolean | NyxNumber | NyxString | NyxDict | nyx_object_list_t:

        ################################################################################################################

        ptr = ffi.lib.nyx_list_get(self.ptr, idx)

        if not ptr:

            raise IndexError(idx)

        ################################################################################################################

        ffi.lib.nyx_object_ref(ptr)

        ################################################################################################################

        try:

            ############################################################################################################

            object_type = ffi.lib.nyx_object_get_type(ptr)

            ############################################################################################################

            if object_type == 200: # NYX_TYPE_NULL
                from .json_null import NyxNull
                return NyxNull(ptr)

            if object_type == 201: # NYX_TYPE_BOOLEAN
                from .json_boolean import NyxBoolean
                return NyxBoolean(ptr)

            if object_type == 202: # NYX_TYPE_NUMBER
                from .json_number import NyxNumber
                return NyxNumber(ptr)

            if object_type == 203: # NYX_TYPE_STRING
                from .json_string import NyxString
                return NyxString(ptr)

            if object_type == 204: # NYX_TYPE_DICT
                from .json_dict import NyxDict
                return NyxDict(ptr)

            if object_type == 205: # NYX_TYPE_LIST
                #### .list import nyx_object_list_t
                return nyx_object_list_t(ptr)

            ############################################################################################################

            raise TypeError(f'internal error, unknown Nyx object type `{object_type}`')

            ############################################################################################################

        except BaseException:

            ffi.lib.nyx_object_unref(ptr)

            raise

    ####################################################################################################################

    def set(self, idx: int, value: obj.NyxObject) -> bool:

        if not isinstance(value, obj.NyxObject):

            raise TypeError('value must be a nyx_object_t')

        return bool(ffi.lib.nyx_list_set(self.ptr, idx, value.ptr))

    ####################################################################################################################

    def push(self, value: obj.NyxObject) -> bool:

        if not isinstance(value, obj.NyxObject):

            raise TypeError('value must be a nyx_object_t')

        return bool(ffi.lib.nyx_list_set(self.ptr, -1, value.ptr))

    ####################################################################################################################

    def size(self) -> int:

        return int(ffi.lib.nyx_list_size(self.ptr))

########################################################################################################################

__all__ = ['nyx_object_list_t']

########################################################################################################################
