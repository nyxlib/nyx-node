# -*- coding: utf-8 -*-
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
    from .json_list import NyxList

########################################################################################################################

class NyxDict(obj.NyxObject):
    """! @brief JSON dict object."""

    ####################################################################################################################

    def __init__(self, ptr = None):
        """! @brief Allocates a new JSON dict object or wraps one.

        @param ptr Optional JSON dict object pointer.
        """

        if ptr is None:

            ptr = bind.lib.nyx_dict_new()

        elif bind.lib.nyx_object_get_type(ptr) != bind.NyxObjectType.DICT:

            raise TypeError('Not a pointer to a Nyx dict object')

        super().__init__(ptr)

    ####################################################################################################################

    def clear(self) -> None:
        """! @brief Clears the content of this JSON dict object."""

        bind.lib.nyx_dict_clear(self.ptr)

    ####################################################################################################################

    def __delitem__(self, key: str) -> None:
        """! @brief Deletes the entry of the provided key.

        @param key Key.
        """

        bind.lib.nyx_dict_del(self.ptr, bind.as_bytes(key, allow_none = False))

    ####################################################################################################################

    def __getitem__(self, key: str) -> NyxNull | NyxBoolean | NyxNumber | NyxString | NyxDict | NyxList:
        """! @brief Gets the JSON object of the provided key.

        @param key Key.
        @return The JSON object.
        """

        ################################################################################################################

        ptr = bind.lib.nyx_dict_get(self.ptr, bind.as_bytes(key, allow_none = False))

        if not ptr:

            raise KeyError(key)

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
                #### .json_dict import NyxDict
                return NyxDict(ptr)

            if object_type == bind.NyxObjectType.LIST:
                from .json_list import NyxList
                return NyxList(ptr)

            ############################################################################################################

            raise TypeError(f'internal error, unknown Nyx object type `{object_type}`')

            ############################################################################################################

        except BaseException:

            bind.lib.nyx_object_unref(ptr)

            raise

    ####################################################################################################################

    def __setitem__(self, key: str, value: obj.NyxObject) -> bool:
        """! @brief Sets a JSON object in this JSON dict object.

        @param key Key.
        @param value JSON object to be added.
        @return `true` if the value was modified, `false` otherwise.
        """

        if not isinstance(value, obj.NyxObject):

            raise TypeError('value must be a Nyx object')

        return bool(bind.lib.nyx_dict_set(self.ptr, bind.as_bytes(key, allow_none = False), value.ptr))

    ####################################################################################################################

    def __len__(self) -> int:
        """! @brief Gets the number of items in this JSON dict object.

        @return The number of items in this JSON dict object.
        """

        return int(bind.lib.nyx_dict_size(self.ptr))

########################################################################################################################

__all__ = ['NyxDict']

########################################################################################################################
