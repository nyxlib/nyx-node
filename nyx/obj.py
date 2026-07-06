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

    def __init__(self, ptr):
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
    def _finalize(ptr) -> None:

        ptr = ctypes.cast(ptr, bind.nyx_object_p)

        ptr.contents.callback = None

        bind.lib.nyx_object_unref(ptr)

    ####################################################################################################################

    @property
    def ptr(self):
        """!
        @brief C pointer to the JSON object.

        @return The JSON object pointer.
        """

        if not self._ptr:

            raise ValueError('Nyx object has been closed')

        return self._ptr

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
        """

        if not callable(callback):

            raise TypeError('Expected a callable')

        ################################################################################################################

        callback_method = getattr(  type(self)  , '_nyx_callback_method', None)

        callback_type = getattr(callback_method, '_nyx_callback_type', None)

        if callback_type is None:

            raise TypeError(f'{type(self).__name__} does not support callbacks')

        ################################################################################################################

        if self._c_callback is None:

            self._c_callback = callback_type(self._nyx_callback_method)

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

    def to_string(self) -> str:
        """!
        @brief Returns a JSON string with special character escaping.

        @return A string that represents this JSON object.
        """

        return bind.take_string(bind.lib.nyx_object_to_string(self.ptr))

    ####################################################################################################################

    def to_cstring(self) -> str:
        """!
        @brief Returns a JSON string without special character escaping.

        @return A string that represents this JSON object.
        """

        return bind.take_string(bind.lib.nyx_object_to_cstring(self.ptr))

    ####################################################################################################################

    def to_xmldoc(self) -> NyxXMLDoc:
        """!
        @brief Converts this JSON Nyx / INDI command to an XML one.

        @return The corresponding XML Nyx / INDI command.
        """

        from .xml import NyxXMLDoc

        return NyxXMLDoc(bind.lib.nyx_object_to_xmldoc(self.ptr))

    ####################################################################################################################

    def __eq__(self, other):

        if not isinstance(other, NyxObject):

            return NotImplemented

        return bool(bind.lib.nyx_object_equal(self.ptr, other.ptr))

    ####################################################################################################################

    def __str__(self):

        return self.to_string()

    def __repr__(self):

        return self.to_string()

########################################################################################################################

__all__ = [name for name in globals() if name.lower().startswith('nyx')]

########################################################################################################################
