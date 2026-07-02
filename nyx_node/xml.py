########################################################################################################################
# NyxNode
# Author: Jérôme ODIER <jerome.odier@lpsc.in2p3.fr>
# SPDX-License-Identifier: GPL-3.0+
########################################################################################################################

from __future__ import annotations

########################################################################################################################

import typing
import weakref

########################################################################################################################

from . import bind

if typing.TYPE_CHECKING:

    from .obj import NyxObject

########################################################################################################################
# XMLDOC                                                                                                               #
########################################################################################################################

class NyxXMLDoc:
    """XML document."""

    ####################################################################################################################

    def __init__(self, ptr):
        """Wraps a C XML document pointer."""

        self._ptr = bind.check_ptr(ptr, 'nyx_xmldoc_t')

        self._finalizer = weakref.finalize(self, NyxXMLDoc._finalize, self._ptr)

    ####################################################################################################################

    @staticmethod
    def _finalize(ptr) -> None:

        bind.lib.nyx_xmldoc_free(ptr)

    ####################################################################################################################

    @property
    def ptr(self):
        """C pointer to the XML document."""

        if not self._ptr:

            raise ValueError('Nyx XMLDoc has been finalized')

        return self._ptr

    ####################################################################################################################

    @staticmethod
    def from_string(string: str) -> NyxXMLDoc:
        """Parses an XML document from a string."""

        from .obj import NyxObject

        return NyxXMLDoc(bind.lib.nyx_xmldoc_parse(bind.as_bytes(string, allow_none = False)))

    ####################################################################################################################

    def to_string(self) -> str:
        """Returns a string representing this XML document."""

        return bind.take_string(bind.lib.nyx_xmldoc_to_string(self.ptr))

    ####################################################################################################################

    def to_json(self) -> NyxObject:
        """Converts this XML Nyx / INDI command to a JSON one."""

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
