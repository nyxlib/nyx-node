# -*- coding: utf-8 -*-
########################################################################################################################
# NyxNode
# Author: Jérôme ODIER <jerome.odier@lpsc.in2p3.fr>
# SPDX-License-Identifier: GPL-3.0+
########################################################################################################################

import ctypes
import typing

########################################################################################################################

from . import bind

########################################################################################################################
# HASH                                                                                                                 #
########################################################################################################################

def nyx_hash(string: str, seed: int) -> int:
    """Hashes a string using the MurmurHash2 algorithm."""

    if not string:

        return int(seed) & 0xFFFFFFFF

    ####################################################################################################################

    data = bind.as_bytes(string, allow_none = False)

    return int(bind.lib.nyx_hash(len(data), data, int(seed) & 0xFFFFFFFF))

########################################################################################################################
# MAC ADDR                                                                                                                 #
########################################################################################################################

def nyx_generate_mac_addr(mac0: int, mac1: int, node_id: str) -> bytes:
    """Generates a MAC address based on a node identifier."""

    ####################################################################################################################

    node_id = bind.as_bytes(node_id, allow_none = False)

    ####################################################################################################################

    result_mac = (bind.c_uint8 * 6)()

    bind.lib.nyx_generate_mac_addr(
        result_mac,
        mac0 & 0xFF,
        mac1 & 0xFF,
        node_id
    )

    return bytes(result_mac)

########################################################################################################################
# BAS64 ENCODE / DECODE                                                                                                                 #
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
