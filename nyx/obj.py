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
import weakref

########################################################################################################################

from . import bind

if typing.TYPE_CHECKING:

    from .node import NyxNode

    from .xml import NyxXMLDoc

########################################################################################################################

def nyx_callback(nyx_callback_type):

    ####################################################################################################################

    def decorate(callback: typing.Callable) -> typing.Callable:

        callback._nyx_callback_type = nyx_callback_type

        return callback

    ####################################################################################################################

    return decorate

########################################################################################################################

class NyxObject:
    """!
    @brief Base class for JSON Nyx objects.
    """

    ####################################################################################################################

    def __init__(self, ptr: int):
        """!
        @brief Wraps a C JSON object pointer.

        @param ptr JSON object pointer.
        """

        self._callbacks = []
        self._c_callback = None

        self._ptr = bind.check_ptr(ptr, 'nyx_object_t')

        self._finalizer = weakref.finalize(self, NyxObject._finalize, self._ptr)

    ####################################################################################################################

    @staticmethod
    def _finalize(ptr: int) -> None:

        ptr = ctypes.cast(ptr, bind.nyx_object_p)

        ptr.contents.callback = None

        bind.lib.nyx_object_unref(ptr)

    ####################################################################################################################

    @property
    def ptr(self) -> int:
        """!
        @private
        @brief C pointer to the JSON object.

        @return The JSON object pointer.
        """

        if not self._ptr:

            raise ValueError('Nyx object has been closed')

        return self._ptr

    ####################################################################################################################

    @staticmethod
    def _wrap_borrowed_ptr(ptr: int) -> NyxObject:

        ################################################################################################################

        ptr = bind.check_ptr(ptr, 'nyx_object_t')

        ################################################################################################################

        bind.lib.nyx_object_ref(ptr)

        ################################################################################################################

        try:

            ############################################################################################################

            object_type = bind.lib.nyx_object_get_type(ptr)

            ############################################################################################################

            if object_type == bind.NyxObjectType.NULL:
                from .json.json_null import NyxNull
                return NyxNull(ptr = ptr)

            if object_type == bind.NyxObjectType.BOOLEAN:
                from .json.json_boolean import NyxBoolean
                return NyxBoolean(ptr = ptr)

            if object_type == bind.NyxObjectType.NUMBER:
                from .json.json_number import NyxNumber
                return NyxNumber(ptr = ptr)

            if object_type == bind.NyxObjectType.STRING:
                from .json.json_string import NyxString
                return NyxString(ptr = ptr)

            if object_type == bind.NyxObjectType.DICT:
                from .json.json_dict import NyxDict
                return NyxDict(ptr = ptr)

            if object_type == bind.NyxObjectType.LIST:
                from .json.json_list import NyxList
                return NyxList(ptr = ptr)

            ############################################################################################################

            raise TypeError(f'internal error, unknown Nyx object type `{object_type}`')

            ############################################################################################################

        except BaseException:

            bind.lib.nyx_object_unref(ptr)

            raise

    ####################################################################################################################

    def _dispatch_callbacks(self, *args):

        return tuple(callback(*args) for callback in tuple(self._callbacks))

    ####################################################################################################################

    # noinspection PyTypeChecker, PyUnresolvedReferences
    def on(self, callback: typing.Callable) -> typing.Callable:
        """!
        @brief Registers a callback triggered when clients modify this object.

        @param callback Callback triggered when clients modify this object.
        @return The registered callback.

        @code{.py}
        @prop.on
        def on_changed(new_value, old_value):
            ...

        @vector.on
        def on_changed(modified):
            ...
        @endcode
        """

        if not callable(callback):

            raise TypeError('Expected a callable')

        ################################################################################################################

        callback_func = getattr(type(self)   , '_nyx_callback_func', None)
        callback_type = getattr(callback_func, '_nyx_callback_type', None)

        if callback_type is None:

            raise TypeError(f'{type(self).__name__} does not support callbacks')

        ################################################################################################################

        if self._c_callback is None:

            self._c_callback = callback_type(self._nyx_callback_func)

            object_ptr = ctypes.cast(self.ptr, bind.nyx_object_p)

            object_ptr.contents.callback = ctypes.cast(
                self._c_callback,
                ctypes.c_void_p,
            )

        ################################################################################################################

        self._callbacks.append(callback)

        ################################################################################################################

        return callback

    ####################################################################################################################

    def notify(self) -> bool:
        """!
        @brief Notifies this Nyx / INDI object to the clients.

        @return @c True if the object was notified, @c False otherwise.
        """

        return bool(bind.lib.nyx_object_notify(self.ptr))

    ####################################################################################################################

    @staticmethod
    def from_string(string: str) -> NyxObject:
        """!
        @brief Parses a JSON object from a string.

        @param string JSON string.
        @return The new JSON object.
        """

        return NyxObject(bind.lib.nyx_object_parse(bind.as_bytes(string, allow_none = False)))

    ####################################################################################################################

    def to_string(self, json_string: bool = True) -> str:
        """!
        @brief Returns a string representing this JSON object.

        @param json_string If @c True, the resulting string is escaped.

        @return A string that represents this JSON document.
        """

        if json_string:
            return bind.take_string(bind.lib.nyx_object_to_string(self.ptr))
        else:
            return bind.take_string(bind.lib.nyx_object_to_cstring(self.ptr))

    ####################################################################################################################

    def to_xmldoc(self) -> NyxXMLDoc:
        """!
        @brief Converts this JSON Nyx / INDI command to the XML one.

        @return The corresponding XML Nyx / INDI command.
        """

        from .xml import NyxXMLDoc

        return NyxXMLDoc(bind.lib.nyx_object_to_xmldoc(self.ptr))

    ####################################################################################################################

    def __eq__(self, other) -> bool:

        if not isinstance(other, NyxObject):

            return NotImplemented

        return bool(bind.lib.nyx_object_equal(self.ptr, other.ptr))

    ####################################################################################################################

    def __str__(self) -> str:

        return self.to_string()

    def __repr__(self) -> str:

        return self.to_string()

########################################################################################################################

__all__ = [name for name in globals() if name.lower().startswith('nyx')]

########################################################################################################################
