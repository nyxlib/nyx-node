# -*- coding: utf-8 -*-
########################################################################################################################
# NyxNode
# Author: Jérôme ODIER <jerome.odier@lpsc.in2p3.fr>
# SPDX-License-Identifier: GPL-3.0+
########################################################################################################################

from __future__ import annotations

import ctypes
import typing

########################################################################################################################

from .. import ffi

########################################################################################################################

def nyx_base64_encode(data: typing.Optional[str | bytes]) -> typing.Optional[str]:

    if not data:

        return None

    ####################################################################################################################

    data = ffi.as_bytes(data, allow_none = False)

    ####################################################################################################################

    result_size = ffi.c_size_t()

    result_buff = ffi.lib.nyx_base64_encode(ctypes.byref(result_size), len(data), data)

    return ffi.take_string(result_buff, result_size.value)

########################################################################################################################

def nyx_base64_decode(data: typing.Optional[str | bytes]) -> typing.Optional[bytes]:

    if not str:

        return None

    ####################################################################################################################

    data = ffi.as_bytes(data, allow_none = False)

    ####################################################################################################################

    result_size = ffi.c_size_t()

    result_buff = ffi.lib.nyx_base64_decode(ctypes.byref(result_size), len(data), data)

    return ffi.take_bytes(result_buff, result_size.value)

########################################################################################################################

__all__ = [name for name in globals() if name.startswith('nyx_') or name.startswith('NYX_')]

########################################################################################################################
