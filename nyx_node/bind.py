# -*- coding: utf-8 -*-
########################################################################################################################
# NyxNode
# Author: Jérôme ODIER <jerome.odier@lpsc.in2p3.fr>
# SPDX-License-Identifier: GPL-3.0+
########################################################################################################################

import os
import sys
import ctypes
import typing
import pathlib

########################################################################################################################
# ERRORS                                                                                                               #
########################################################################################################################

class NyxError(RuntimeError):
    """Base error raised by the Python Nyx binding."""

class NyxLibraryError(NyxError):
    """Raised when the C shared library cannot be loaded or used."""

########################################################################################################################
# ALIASES                                                                                                              #
########################################################################################################################

c_void_p = ctypes.c_void_p
c_char_p = ctypes.c_char_p
c_bool = ctypes.c_bool
c_uint = ctypes.c_uint
c_int = ctypes.c_int
c_ulong = ctypes.c_ulong
c_long = ctypes.c_long

c_uint8 = ctypes.c_uint8
c_int8 = ctypes.c_int8
c_uint32 = ctypes.c_uint32
c_int32 = ctypes.c_int32
c_uint64 = ctypes.c_uint64
c_int64 = ctypes.c_int64
c_size_t = ctypes.c_size_t

c_float = ctypes.c_float
c_double = ctypes.c_double

########################################################################################################################
# CALLBACKS                                                                                                            #
########################################################################################################################

class nyx_dict_t(ctypes.Structure):

    pass

########################################################################################################################

nyx_dict_p = ctypes.POINTER(nyx_dict_t)

########################################################################################################################

nyx_callback_int_t = ctypes.CFUNCTYPE(
    c_bool,
    nyx_dict_p,
    nyx_dict_p,
    c_int,
    c_int,
)

nyx_callback_uint_t = ctypes.CFUNCTYPE(
    c_bool,
    nyx_dict_p,
    nyx_dict_p,
    c_uint,
    c_uint,
)

nyx_callback_long_t = ctypes.CFUNCTYPE(
    c_bool,
    nyx_dict_p,
    nyx_dict_p,
    c_long,
    c_long,
)

nyx_callback_ulong_t = ctypes.CFUNCTYPE(
    c_bool,
    nyx_dict_p,
    nyx_dict_p,
    c_ulong,
    c_ulong,
)

nyx_callback_double_t = ctypes.CFUNCTYPE(
    c_bool,
    nyx_dict_p,
    nyx_dict_p,
    c_double,
    c_double,
)

nyx_callback_str_t = ctypes.CFUNCTYPE(
    c_bool,
    nyx_dict_p,
    nyx_dict_p,
    c_char_p,
    c_char_p,
)

nyx_callback_buffer_t = ctypes.CFUNCTYPE(
    c_bool,
    nyx_dict_p,
    nyx_dict_p,
    c_size_t,
    c_void_p,
)

nyx_callback_vector_t = ctypes.CFUNCTYPE(
    None,
    nyx_dict_p,
    c_bool,
)

########################################################################################################################

class nyx_object_t(ctypes.Structure):

    _fields_ = [
        ('type', c_int32),
        ('flags', c_uint64),
        ('ref', c_int32),
        ('node', c_void_p),
        ('parent', c_void_p),
        ('callback', c_void_p),
        ('ctx', c_void_p),
    ]

########################################################################################################################

nyx_dict_t._fields_ = [
    ('base', nyx_object_t),
]

########################################################################################################################
# HELPERS                                                                                                              #
########################################################################################################################

def as_bytes(value: str | bytes | bytearray | memoryview | None, *, allow_none: bool = True) -> bytes | None:

    ####################################################################################################################

    if value is None:

        if not allow_none:

            raise TypeError('None is not allowed here')

        return None

    ####################################################################################################################

    if isinstance(value, (bytes, bytearray, memoryview)):

        return bytes(value)

    if isinstance(value, str):

        return value.encode('utf-8')

    ####################################################################################################################

    raise TypeError(f'expected str or bytes, got {type(value).__name__}')

########################################################################################################################

def check_ptr(ptr: int | c_void_p | None, what: str = 'C object') -> c_void_p:

    ####################################################################################################################

    if isinstance(ptr, c_void_p):

        if ptr.value:

            return ptr

    elif ptr:

        # noinspection PyTypeChecker
        return c_void_p(ptr)

    ####################################################################################################################

    raise TypeError(f'{what} is NULL')

########################################################################################################################

def take_bytes(ptr: int | c_void_p | None, size: int) -> bytes:

    cptr = check_ptr(ptr, 'C buffer')

    try:
        return ctypes.string_at(cptr, size)
    finally:
        lib.nyx_memory_free(cptr)

########################################################################################################################

def take_string(ptr: int | c_void_p | None, size: int | None = None) -> str:

    cptr = check_ptr(ptr, 'C string')

    try:
        if size is None:
            return ctypes.string_at(cptr).decode('utf-8')
        else:
            return ctypes.string_at(cptr, size).decode('utf-8')
    finally:
        lib.nyx_memory_free(cptr)

########################################################################################################################
# LOAD LIBRARY                                                                                                         #
########################################################################################################################

def _load_library() -> ctypes.CDLL:

    ####################################################################################################################

    if os.name == 'nt':
        names = ('libnyx-node.dll', 'nyx-node.dll')
        directories = ()
    elif sys.platform == 'darwin':
        names = ('libnyx-node.dylib', 'nyx-node.dylib')
        directories = (
            pathlib.Path('/usr/local/lib'),
            pathlib.Path('/usr/lib'),
            pathlib.Path('/lib'),
        )
    else:
        names = ('libnyx-node.so', 'nyx-node.so')
        directories = (
            pathlib.Path('/usr/local/lib'),
            pathlib.Path('/usr/lib'),
            pathlib.Path('/lib'),
        )

    ####################################################################################################################

    candidates = (
        *(str(directory / name) for directory in directories for name in names),
        *names,
    )

    print(candidates)

    ####################################################################################################################

    errors: list[str] = []

    for candidate in dict.fromkeys(filter(None, candidates)):

        if not os.path.dirname(candidate) or pathlib.Path(candidate).exists():

            try:

                return ctypes.CDLL(candidate)

            except OSError as exc:

                errors.append(f'{candidate}: {exc}')

    ####################################################################################################################

    message = 'Unable to load libnyx-node. Install it in a system library directory.'

    details = '\n'.join(errors)

    if details:

        message += f'\nTried:\n{details}'

    ####################################################################################################################

    raise NyxLibraryError(message)

########################################################################################################################

lib = _load_library()

########################################################################################################################
# BINDINGS                                                                                                             #
########################################################################################################################

def _bind(name: str, restype, argtypes: typing.Sequence[object]) -> None:

    ####################################################################################################################

    argtypes = list(argtypes)

    ####################################################################################################################

    try:

        func = getattr(lib, name)

        func.argtypes = argtypes
        func.restype = restype

    except AttributeError as e:

        raise NyxLibraryError(f'Missing C symbol: {name}') from e

########################################################################################################################

## UTILS ##

_bind('nyx_hash', ctypes.c_uint32, [c_size_t, c_void_p, c_uint32])

_bind('nyx_generate_mac_addr', None, [c_uint8 * 6, c_uint8, c_uint8, c_char_p])

_bind('nyx_base64_encode', c_void_p, [ctypes.POINTER(c_size_t), c_size_t, c_void_p])
_bind('nyx_base64_decode', c_void_p, [ctypes.POINTER(c_size_t), c_size_t, c_void_p])

########################################################################################################################

## OBJECT ##

_bind('nyx_object_parse', c_void_p, [c_char_p])
_bind('nyx_object_ref', None, [c_void_p])
_bind('nyx_object_unref', None, [c_void_p])
_bind('nyx_object_get_type', c_int32, [c_void_p])
_bind('nyx_object_to_string', c_char_p, [c_void_p])
_bind('nyx_object_to_cstring', c_char_p, [c_void_p])

########################################################################################################################

## XMLDOC ##

_bind('nyx_xmldoc_parse', c_void_p, [c_char_p])
_bind('nyx_xmldoc_free_recursive', None, [c_void_p])
_bind('nyx_xmldoc_to_string', c_char_p, [c_void_p])

########################################################################################################################

## JSON ##

_bind("nyx_null_new", c_void_p, [])

_bind("nyx_boolean_new", c_void_p, [])
_bind("nyx_boolean_get", c_bool, [c_void_p])
_bind("nyx_boolean_set", c_bool, [c_void_p, c_bool])

_bind("nyx_string_new", c_void_p, [])
_bind('nyx_string_get', c_char_p, [c_void_p])
_bind("nyx_string_set", c_bool, [c_void_p, c_char_p, c_bool])

_bind('nyx_number_new', c_void_p, [])
_bind('nyx_number_get', c_double, [c_void_p])
_bind('nyx_number_set', c_bool, [c_void_p, c_double])

_bind('nyx_dict_new', c_void_p, [])
_bind('nyx_dict_clear', None, [c_void_p])
_bind('nyx_dict_del', None, [c_void_p, c_char_p])
_bind('nyx_dict_get', c_void_p, [c_void_p, c_char_p])
_bind('nyx_dict_set', c_bool, [c_void_p, c_char_p, c_void_p])
_bind('nyx_dict_size', c_size_t, [c_void_p])

_bind('nyx_list_new', c_void_p, [])
_bind('nyx_list_clear', None, [c_void_p])
_bind('nyx_list_del', None, [c_void_p, c_size_t])
_bind('nyx_list_get', c_void_p, [c_void_p, c_size_t])
_bind('nyx_list_set', c_bool, [c_void_p, c_size_t, c_void_p])
_bind('nyx_list_size', c_size_t, [c_void_p])

########################################################################################################################
