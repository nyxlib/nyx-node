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

class NyxList(obj.NyxObject):
    """!
    @brief JSON list object.
    """

    ####################################################################################################################

    def __init__(self, ptr = None):
        """!
        @brief Allocates a new JSON list object or wraps one.

        @param ptr Optional JSON list object pointer.
        """

        if ptr is None:

            ptr = bind.lib.nyx_list_new()

        elif bind.lib.nyx_object_get_type(ptr) != bind.NyxObjectType.LIST:

            raise TypeError('Not a pointer to a Nyx list object')

        super().__init__(ptr)

    ####################################################################################################################

    def clear(self) -> None:
        """!
        @brief Clears the content of this JSON list object.

        @return None
        """

        bind.lib.nyx_list_clear(self.ptr)

    ####################################################################################################################

    def __delitem__(self, idx: int) -> None:
        """!
        @brief Deletes the entry at the provided index.

        @param idx Index.
        @return None
        """

        bind.lib.nyx_list_del(self.ptr, idx)

    ####################################################################################################################

    def __getitem__(self, idx: int) -> obj.NyxObject:
        """!
        @brief Gets the JSON object at the provided index.

        @param idx Index.
        @return The JSON object at the provided index.
        """

        ################################################################################################################

        ptr = bind.lib.nyx_list_get(self.ptr, idx)

        if not ptr:

            raise IndexError(idx)

        ################################################################################################################

        return obj.NyxObject._wrap_borrowed_ptr(ptr)

    ####################################################################################################################

    def __setitem__(self, idx: int, value: obj.NyxObject) -> bool:
        """!
        @brief Sets a JSON object at the provided index.

        @param idx Index.
        @param value JSON object to be added.
        @return @c True if the value was modified, @c False otherwise.
        """

        if not isinstance(value, obj.NyxObject):

            raise TypeError('value must be a Nyx object')

        return bool(bind.lib.nyx_list_set(self.ptr, idx, value.ptr))

    ####################################################################################################################

    def append(self, value: obj.NyxObject) -> bool:
        """!
        @brief Appends a JSON object in this JSON list object.

        @param value JSON object to be added.
        @return @c True if the value was modified, @c False otherwise.
        """

        return self.__setitem__(-1, value)

    ####################################################################################################################

    def __len__(self) -> int:
        """!
        @brief Gets the number of items in this JSON list object.

        @return The number of items.
        """

        return int(bind.lib.nyx_list_size(self.ptr))

    ####################################################################################################################

    def _iterate(self) -> typing.Iterator[typing.Tuple[int, int]]:

        ################################################################################################################

        list_ptr = ctypes.cast(self.ptr, bind.nyx_list_p)

        iterator = bind.nyx_list_iter_t(
            0,
            list_ptr.contents.head,
        )

        ################################################################################################################

        idx = bind.c_size_t()
        val = bind.c_void_p()

        ################################################################################################################

        while bind.lib.nyx_list_iterate(
            ctypes.byref(iterator),
            ctypes.byref(idx),
            ctypes.byref(val),
        ):

            if idx.value is None\
               or               \
               val.value is None:

                raise RuntimeError('Invalid Nyx list iterator result')

            yield idx.value, val.value

    ####################################################################################################################

    def __iter__(self) -> typing.Iterator[obj.NyxObject]:
        """!
        @brief Iterates over the values of this JSON list object.

        @return An iterator over the values.
        """

        for _, ptr in self._iterate():

            yield obj.NyxObject._wrap_borrowed_ptr(ptr)

    ####################################################################################################################

    def indices(self) -> typing.Iterator[int]:
        """!
        @brief Iterates over the indices of this JSON list object.

        @return An iterator over the indices.
        """

        for idx, _ in self._iterate():

            yield idx

    ####################################################################################################################

    def values(self) -> typing.Iterator[obj.NyxObject]:
        """!
        @brief Iterates over the values of this JSON list object.

        @return An iterator over the values.
        """

        for _, ptr in self._iterate():

            yield obj.NyxObject._wrap_borrowed_ptr(ptr)

    ####################################################################################################################

    def items(self) -> typing.Iterator[typing.Tuple[int, obj.NyxObject]]:
        """!
        @brief Iterates over the index/value pairs of this JSON list object.

        @return An iterator over the index/value pairs.
        """

        for idx, ptr in self._iterate():

            yield idx, obj.NyxObject._wrap_borrowed_ptr(ptr)

########################################################################################################################

__all__ = ['NyxList']

########################################################################################################################
