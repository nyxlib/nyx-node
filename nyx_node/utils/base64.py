# -*- coding: utf-8 -*-
########################################################################################################################
# NyxNode
# Author: Jérôme ODIER <jerome.odier@lpsc.in2p3.fr>
# SPDX-License-Identifier: GPL-3.0+
########################################################################################################################

import ctypes
import typing

########################################################################################################################

from .. import bind

########################################################################################################################

def nyx_base64_encode(data: typing.Optional[str | bytes]) -> typing.Optional[str]:
    """Encodes a buffer using the Base64 algorithm."""

    if not data:

        return None

    ####################################################################################################################

    data = bind.as_bytes(data, allow_none = False)

    ####################################################################################################################

    result_size = bind.c_size_t()

    result_buff = bind.lib.nyx_base64_encode(ctypes.byref(result_size), len(data), data)

    return bind.take_string(result_buff, result_size.value)

########################################################################################################################

def nyx_base64_decode(data: typing.Optional[str | bytes]) -> typing.Optional[bytes]:
    """Decodes a string using the Base64 algorithm."""

    if not str:

        return None

    ####################################################################################################################

    data = bind.as_bytes(data, allow_none = False)

    ####################################################################################################################

    result_size = bind.c_size_t()

    result_buff = bind.lib.nyx_base64_decode(ctypes.byref(result_size), len(data), data)

    return bind.take_bytes(result_buff, result_size.value)

########################################################################################################################

__all__ = [name for name in globals() if name.lower().startswith('nyx')]

########################################################################################################################
