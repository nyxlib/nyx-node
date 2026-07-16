# -*- coding: utf-8 -*-
########################################################################################################################
# NyxNode
# Author: Jérôme ODIER <jerome.odier@lpsc.in2p3.fr>
# SPDX-License-Identifier: GPL-3.0+
########################################################################################################################

from __future__ import annotations

########################################################################################################################

import ctypes
import typing

########################################################################################################################

from .. import bind
from .. import obj

########################################################################################################################

class NyxDict(obj.NyxObject):
    """!
    @brief JSON dict object.
    """

    ####################################################################################################################

    def __init__(self, ptr = None):
        """!
        @brief Allocates a new JSON dict object or wraps one.

        @param ptr Optional JSON dict object pointer.
        """

        if ptr is None:

            ptr = bind.lib.nyx_dict_new()

        elif bind.lib.nyx_object_get_type(ptr) != bind.NyxObjectType.DICT:

            raise TypeError('Not a pointer to a Nyx dict object')

        super().__init__(ptr)

    ####################################################################################################################

    def clear(self) -> None:
        """!
        @brief Clears the content of this JSON dict object.
        @return None
        """

        bind.lib.nyx_dict_clear(self.ptr)

    ####################################################################################################################

    def __delitem__(self, key: str) -> None:
        """!
        @brief Deletes the entry of the provided key.

        @param key Key.
        @return None
        """

        bind.lib.nyx_dict_del(self.ptr, bind.as_bytes(key, allow_none = False))

    ####################################################################################################################

    def __getitem__(self, key: str) -> obj.NyxObject:
        """!
        @brief Gets the JSON object of the provided key.

        @param key Key.
        @return The JSON object.
        """

        ################################################################################################################

        ptr = bind.lib.nyx_dict_get(self.ptr, bind.as_bytes(key, allow_none = False))

        if not ptr:

            raise KeyError(key)

        ################################################################################################################

        return obj.NyxObject._wrap_borrowed_ptr(ptr)

    ####################################################################################################################

    def __setitem__(self, key: str, value: obj.NyxObject) -> bool:
        """!
        @brief Sets a JSON object in this JSON dict object.

        @param key Key.
        @param value JSON object to be added.
        @return @c True if the value was modified, @c False otherwise.
        """

        if not isinstance(value, obj.NyxObject):

            raise TypeError('value must be a Nyx object')

        return bool(bind.lib.nyx_dict_set(self.ptr, bind.as_bytes(key, allow_none = False), value.ptr))

    ####################################################################################################################

    def __len__(self) -> int:
        """!
        @brief Gets the number of items in this JSON dict object.

        @return The number of items.
        """

        return int(bind.lib.nyx_dict_size(self.ptr))

    ####################################################################################################################

    def _iterate(self) -> typing.Iterator[typing.Tuple[str, int]]:

        ################################################################################################################

        dict_ptr = ctypes.cast(self.ptr, bind.nyx_dict_p)

        iterator = bind.nyx_dict_iter_t(
            0,
            dict_ptr.contents.head,
        )

        ################################################################################################################

        key = bind.c_char_p()
        val = bind.c_void_p()

        ################################################################################################################

        while bind.lib.nyx_dict_iterate(
            ctypes.byref(iterator),
            ctypes.byref(key),
            ctypes.byref(val),
        ):

            if key.value is None\
               or               \
               val.value is None:

                raise RuntimeError('Invalid Nyx dict iterator result')

            yield key.value.decode('utf-8'), val.value

    ####################################################################################################################

    def __iter__(self) -> typing.Iterator[str]:

        for key, _ in self._iterate():

            yield key

    ####################################################################################################################

    def keys(self) -> typing.Iterator[str]:
        """!
        @brief Iterates over the keys of this JSON dict object.

        @return An iterator over the keys.
        """

        for key, _ in self._iterate():

            yield key

    ####################################################################################################################

    def values(self) -> typing.Iterator[obj.NyxObject]:
        """!
        @brief Iterates over the values of this JSON dict object.

        @return An iterator over the values.
        """

        for _, ptr in self._iterate():

            yield obj.NyxObject._wrap_borrowed_ptr(ptr)

    ####################################################################################################################

    def items(self) -> typing.Iterator[typing.Tuple[str, obj.NyxObject]]:
        """!
        @brief Iterates over the key/value pairs of this JSON dict object.

        @return An iterator over the key/value pairs.
        """

        for key, ptr in self._iterate():

            yield key, obj.NyxObject._wrap_borrowed_ptr(ptr)

########################################################################################################################

__all__ = ['NyxDict']

########################################################################################################################
