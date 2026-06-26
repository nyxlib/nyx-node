########################################################################################################################
# NyxNode
# Author: Jérôme ODIER <jerome.odier@lpsc.in2p3.fr>
# SPDX-License-Identifier: GPL-3.0+
########################################################################################################################

from __future__ import annotations

import weakref

########################################################################################################################

from . import ffi

########################################################################################################################
# OBJECT                                                                                                               #
########################################################################################################################

class NyxObject:

    ####################################################################################################################

    def __init__(self, ptr):

        self._ptr = ffi.check_ptr(ptr, 'nyx_object_t')

        self._finalizer = weakref.finalize(self, ffi.lib.nyx_object_unref, self._ptr)

    ####################################################################################################################

    def close(self) -> None:

        if self._finalizer.alive:

            self._ptr = None

            self._finalizer()

    ####################################################################################################################

    @property
    def ptr(self):

        if not self._ptr:

            raise ValueError('Nyx object has been closed')

        return self._ptr

    ####################################################################################################################

    @staticmethod
    def from_string(string: str) -> NyxObject:

        return NyxObject(ffi.lib.nyx_object_parse(ffi.as_bytes(string, allow_none = False)))

    ####################################################################################################################

    def to_string(self) -> str:

        return ffi.lib.nyx_object_to_string(self.ptr).decode('utf-8')

    ####################################################################################################################

    def to_cstring(self) -> str:

        return ffi.lib.nyx_object_to_cstring(self.ptr).decode('utf-8')

    ####################################################################################################################

    def to_xmldoc(self, object: NyxObject) -> NyxXMLDoc:

        return NyxXMLDoc(ffi.lib.nyx_object_to_xmldoc(object.ptr))

    ####################################################################################################################

    def __enter__(self) -> NyxObject:

        return self

    ####################################################################################################################

    def __exit__(self, exc_type, exc_value, traceback) -> None:

        self.close()

    ####################################################################################################################

    def __eq__(self, other):

        if not isinstance(other, NyxObject):

            return NotImplemented

        return bool(ffi.lib.nyx_object_equal(self.ptr, other.ptr))

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

        self._ptr = ffi.check_ptr(ptr, 'nyx_xmldoc_t')

        self._finalizer = weakref.finalize(self, ffi.lib.nyx_xmldoc_free_recursive, self._ptr)

    ####################################################################################################################

    def close(self) -> None:

        if self._finalizer.alive:

            self._ptr = None

            self._finalizer()

    ####################################################################################################################

    @property
    def ptr(self):

        if not self._ptr:

            raise ValueError('Nyx XMLDoc has been closed')

        return self._ptr

    ####################################################################################################################

    @staticmethod
    def from_string(string: str) -> NyxXMLDoc:

        return NyxXMLDoc(ffi.lib.nyx_xmldoc_parse(ffi.as_bytes(string, allow_none = False)))

    ####################################################################################################################

    def to_string(self) -> str:

        return ffi.lib.nyx_xmldoc_to_string(self.ptr).decode('utf-8')

    ####################################################################################################################

    def to_json(self, xmldoc: NyxXMLDoc) -> NyxObject:

        return NyxObject(ffi.lib.nyx_xmldoc_to_object(xmldoc.ptr))

    ####################################################################################################################

    def __enter__(self) -> NyxXMLDoc:

        return self

    ####################################################################################################################

    def __exit__(self, exc_type, exc_value, traceback) -> None:

        self.close()

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
