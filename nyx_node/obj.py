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
import traceback

########################################################################################################################

from . import bind

########################################################################################################################
# CALLBACKS                                                                                                            #
########################################################################################################################

# noinspection PyBroadException
@bind.nyx_callback_int_t
def c_callback_int(vector, prop, new_value, old_value):

    try:

        self = ctypes.cast(ctypes.c_void_p(prop.contents.base.ctx), ctypes.POINTER(ctypes.py_object)).contents.value

        return bool(self._callback(vector, prop, new_value, old_value))

    except BaseException:

        traceback.print_exc()

        return False

########################################################################################################################

# noinspection PyBroadException
@bind.nyx_callback_uint_t
def c_callback_uint(vector, prop, new_value, old_value):

    try:

        self = ctypes.cast(ctypes.c_void_p(prop.contents.base.ctx), ctypes.POINTER(ctypes.py_object)).contents.value

        return bool(self._callback(vector, prop, new_value, old_value))

    except BaseException:

        traceback.print_exc()

        return False

########################################################################################################################

# noinspection PyBroadException
@bind.nyx_callback_long_t
def c_callback_long(vector, prop, new_value, old_value):

    try:

        self = ctypes.cast(ctypes.c_void_p(prop.contents.base.ctx), ctypes.POINTER(ctypes.py_object)).contents.value

        return bool(self._callback(vector, prop, new_value, old_value))

    except BaseException:

        traceback.print_exc()

        return False

########################################################################################################################

# noinspection PyBroadException
@bind.nyx_callback_ulong_t
def c_callback_ulong(vector, prop, new_value, old_value):

    try:

        self = ctypes.cast(ctypes.c_void_p(prop.contents.base.ctx), ctypes.POINTER(ctypes.py_object)).contents.value

        return bool(self._callback(vector, prop, new_value, old_value))

    except BaseException:

        traceback.print_exc()

        return False

########################################################################################################################

# noinspection PyBroadException
@bind.nyx_callback_double_t
def c_callback_double(vector, prop, new_value, old_value):

    try:

        self = ctypes.cast(ctypes.c_void_p(prop.contents.base.ctx), ctypes.POINTER(ctypes.py_object)).contents.value

        return bool(self._callback(vector, prop, new_value, old_value))

    except BaseException:

        traceback.print_exc()

        return False

########################################################################################################################

# noinspection PyBroadException
@bind.nyx_callback_str_t
def c_callback_str(vector, prop, new_value, old_value):

    new_value = new_value.decode('utf-8') if new_value is not None else None
    old_value = old_value.decode('utf-8') if old_value is not None else None

    try:

        self = ctypes.cast(ctypes.c_void_p(prop.contents.base.ctx), ctypes.POINTER(ctypes.py_object)).contents.value

        return bool(self._callback(vector, prop, new_value, old_value))

    except BaseException:

        traceback.print_exc()

        return False

########################################################################################################################

# noinspection PyBroadException
@bind.nyx_callback_buffer_t
def c_callback_buffer(vector, prop, size, buff):

    new_value = ctypes.string_at(buff, size) if buff is not None and size > 0 else b''

    try:

        self = ctypes.cast(ctypes.c_void_p(prop.contents.base.ctx), ctypes.POINTER(ctypes.py_object)).contents.value

        return bool(self._callback(vector, prop, new_value))

    except BaseException:

        traceback.print_exc()

        return False

########################################################################################################################

# noinspection PyBroadException
@bind.nyx_callback_vector_t
def c_callback_vector(vector, modified):

    try:

        self = ctypes.cast(ctypes.c_void_p(vector.contents.base.ctx), ctypes.POINTER(ctypes.py_object)).contents.value

        self._callback(vector, bool(modified))

    except BaseException:

        traceback.print_exc()

########################################################################################################################
# OBJECT                                                                                                               #
########################################################################################################################

class NyxObject:

    ####################################################################################################################

    _PROPERTY_CALLBACKS: dict[str, typing.Any] = {
        'int': c_callback_int,
        'uint': c_callback_uint,
        'long': c_callback_long,
        'ulong': c_callback_ulong,
        'double': c_callback_double,
        'str': c_callback_str,
        'buffer': c_callback_buffer,
        'vector': c_callback_vector,
    }

    ####################################################################################################################

    def __init__(self, ptr):

        self._callback = None
        self._ctx = None

        self._ptr = bind.check_ptr(ptr, 'nyx_object_t')

        self._finalizer = weakref.finalize(self, NyxObject._finalize, self._ptr)

    ####################################################################################################################

    @staticmethod
    def _finalize(ptr) -> None:

        ptr = ctypes.cast(ptr, bind.nyx_object_p)

        ptr.contents.callback = None
        ptr.contents.   ctx   = None

        bind.lib.nyx_object_unref(ptr)

    ####################################################################################################################

    @property
    def ptr(self):

        if not self._ptr:

            raise ValueError('Nyx object has been closed')

        return self._ptr

    ####################################################################################################################

    def clear_callback(self) -> None:

        object_ptr = ctypes.cast(self.ptr, ctypes.POINTER(bind.nyx_object_t))

        object_ptr.contents.callback = None
        object_ptr.contents.   ctx   = None

        self._callback = None
        self._ctx = None

    ####################################################################################################################

    def set_callback(self, kind: str, callback) -> None:

        ################################################################################################################

        try:

            c_callback = self._PROPERTY_CALLBACKS[kind]

        except KeyError:

            raise ValueError(f'Invalid Nyx callback type: {kind!r}') from None

        ################################################################################################################

        self.clear_callback()

        ################################################################################################################

        if callback is not None:

            ############################################################################################################

            self._callback = callback
            self._ctx = ctypes.py_object(self)

            ############################################################################################################

            object_ptr = ctypes.cast(self.ptr, ctypes.POINTER(bind.nyx_object_t))

            object_ptr.contents.callback = ctypes.cast(c_callback, ctypes.c_void_p)

            object_ptr.contents.ctx = ctypes.addressof(self._ctx)

    ####################################################################################################################

    @staticmethod
    def from_string(string: str) -> NyxObject:

        return NyxObject(bind.lib.nyx_object_parse(bind.as_bytes(string, allow_none = False)))

    ####################################################################################################################

    def to_string(self) -> str:

        return bind.take_string(bind.lib.nyx_object_to_string(self.ptr))

    ####################################################################################################################

    def to_cstring(self) -> str:

        return bind.take_string(bind.lib.nyx_object_to_cstring(self.ptr))

    ####################################################################################################################

    def to_xmldoc(self) -> NyxXMLDoc:

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
# XMLDOC                                                                                                               #
########################################################################################################################

class NyxXMLDoc:

    ####################################################################################################################

    def __init__(self, ptr):

        self._ptr = bind.check_ptr(ptr, 'nyx_xmldoc_t')

        self._finalizer = weakref.finalize(self, NyxXMLDoc._finalize, self._ptr)

    ####################################################################################################################

    @staticmethod
    def _finalize(ptr) -> None:

        bind.lib.nyx_xmldoc_free(ptr)

    ####################################################################################################################

    @property
    def ptr(self):

        if not self._ptr:

            raise ValueError('Nyx XMLDoc has been closed')

        return self._ptr

    ####################################################################################################################

    @staticmethod
    def from_string(string: str) -> NyxXMLDoc:

        return NyxXMLDoc(bind.lib.nyx_xmldoc_parse(bind.as_bytes(string, allow_none = False)))

    ####################################################################################################################

    def to_string(self) -> str:

        return bind.take_string(bind.lib.nyx_xmldoc_to_string(self.ptr))

    ####################################################################################################################

    def to_json(self) -> NyxObject:

        return NyxObject(bind.lib.nyx_xmldoc_to_object(self.ptr))

    ####################################################################################################################

    def __eq__(self, other):

        if not isinstance(other, NyxXMLDoc):

            return NotImplemented

        return self.to_string() == other.to_string()

    ####################################################################################################################

    def __str__(self):

        return self.to_string()

    def __repr__(self):

        return self.to_string()

########################################################################################################################

__all__ = [name for name in globals() if name.lower().startswith('nyx')]

########################################################################################################################
