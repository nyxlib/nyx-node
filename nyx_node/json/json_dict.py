########################################################################################################################
# NyxNode
# Author: Jérôme ODIER <jerome.odier@lpsc.in2p3.fr>
# SPDX-License-Identifier: GPL-3.0+
########################################################################################################################

from __future__ import annotations

import typing

########################################################################################################################

from .. import ffi
from .. import obj

if typing.TYPE_CHECKING:

    from .json_null import nyx_object_null_t
    from .json_boolean import nyx_object_boolean_t
    from .json_number import nyx_object_number_t
    from .json_string import nyx_object_string_t
    from .json_list import nyx_object_list_t

########################################################################################################################

class nyx_object_dict_t(obj.nyx_object_t):

    ####################################################################################################################

    def __init__(self, ptr = None):

        if ptr is None:

            ptr = ffi.lib.nyx_dict_new()

        super().__init__(ptr)

    ####################################################################################################################

    def clear(self) -> None:

        ffi.lib.nyx_dict_clear(self.ptr)

    ####################################################################################################################

    def delete(self, key: str) -> None:

        ffi.lib.nyx_dict_del(self.ptr, ffi.as_bytes(key, allow_none = False))

    ####################################################################################################################

    def get(self, key: str) -> nyx_object_null_t | nyx_object_boolean_t | nyx_object_number_t | nyx_object_string_t | nyx_object_dict_t | nyx_object_list_t:

        ################################################################################################################

        ptr = ffi.lib.nyx_dict_get(self.ptr, ffi.as_bytes(key, allow_none = False))

        if not ptr:

            raise KeyError(key)

        ################################################################################################################

        ffi.lib.nyx_object_ref(ptr)

        ################################################################################################################

        try:

            ############################################################################################################

            object_type = ffi.lib.nyx_object_get_type(ptr)

            ############################################################################################################

            if object_type == 200: # NYX_TYPE_NULL
                from .json_null import nyx_object_null_t
                return nyx_object_null_t(ptr)

            if object_type == 201: # NYX_TYPE_BOOLEAN
                from .json_boolean import nyx_object_boolean_t
                return nyx_object_boolean_t(ptr)

            if object_type == 202: # NYX_TYPE_NUMBER
                from .json_number import nyx_object_number_t
                return nyx_object_number_t(ptr)

            if object_type == 203: # NYX_TYPE_STRING
                from .json_string import nyx_object_string_t
                return nyx_object_string_t(ptr)

            if object_type == 204: # NYX_TYPE_DICT
                #### .dict import nyx_object_dict_t
                return nyx_object_dict_t(ptr)

            if object_type == 205: # NYX_TYPE_LIST
                from .json_list import nyx_object_list_t
                return nyx_object_list_t(ptr)

            ############################################################################################################

            raise TypeError(f'internal error, unknown Nyx object type `{object_type}`')

            ############################################################################################################

        except BaseException:

            ffi.lib.nyx_object_unref(ptr)

            raise

    ####################################################################################################################

    def set(self, key: str, value: obj.nyx_object_t) -> bool:

        if not isinstance(value, obj.nyx_object_t):

            raise TypeError('value must be a nyx_object_t')

        return bool(ffi.lib.nyx_dict_set(self.ptr, ffi.as_bytes(key, allow_none = False), value.ptr))

    ####################################################################################################################

    def size(self) -> int:

        return int(ffi.lib.nyx_dict_size(self.ptr))

########################################################################################################################

__all__ = ['nyx_object_dict_t']

########################################################################################################################
