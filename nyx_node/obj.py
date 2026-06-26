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

class nyx_object_t:

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
    def from_string(string: str) -> nyx_object_t:

        return nyx_object_t(ffi.lib.nyx_object_parse(ffi.as_bytes(string, allow_none = False)))

    ####################################################################################################################

    def to_string(self) -> str:

        return ffi.lib.nyx_object_to_string(self.ptr).decode('utf-8')

    ####################################################################################################################

    def to_cstring(self) -> str:

        return ffi.lib.nyx_object_to_cstring(self.ptr).decode('utf-8')

    ####################################################################################################################

    def to_xmldoc(self, object: nyx_object_t) -> nyx_xmldoc_t:

        return nyx_xmldoc_t(ffi.lib.nyx_object_to_xmldoc(object.ptr))

    ####################################################################################################################

    def __enter__(self) -> nyx_object_t:

        return self

    ####################################################################################################################

    def __exit__(self, exc_type, exc_value, traceback) -> None:

        self.close()

    ####################################################################################################################

    def __eq__(self, other):

        if not isinstance(other, nyx_object_t):

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

class nyx_xmldoc_t:

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
    def from_string(string: str) -> nyx_xmldoc_t:

        return nyx_xmldoc_t(ffi.lib.nyx_xmldoc_parse(ffi.as_bytes(string, allow_none = False)))

    ####################################################################################################################

    def to_string(self) -> str:

        return ffi.lib.nyx_xmldoc_to_string(self.ptr).decode('utf-8')

    ####################################################################################################################

    def to_json(self, xmldoc: nyx_xmldoc_t) -> nyx_object_t:

        return nyx_object_t(ffi.lib.nyx_xmldoc_to_object(xmldoc.ptr))

    ####################################################################################################################

    def __enter__(self) -> nyx_xmldoc_t:

        return self

    ####################################################################################################################

    def __exit__(self, exc_type, exc_value, traceback) -> None:

        self.close()

    ####################################################################################################################

    def __eq__(self, other):

        if not isinstance(other, nyx_xmldoc_t):

            return NotImplemented

        return self.to_string() == other.to_string()

    ####################################################################################################################

    def __str__(self):

        return self.to_string()

    def __repr__(self):

        return self.to_string()

########################################################################################################################

__all__ = [name for name in globals() if name.startswith('nyx_') or name.startswith('NYX_')]

########################################################################################################################
