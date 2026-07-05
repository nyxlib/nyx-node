# -*- coding: utf-8 -*-
########################################################################################################################
# NyxNode
# Author: Jérôme ODIER <jerome.odier@lpsc.in2p3.fr>
# SPDX-License-Identifier: GPL-3.0+
########################################################################################################################

import typing

########################################################################################################################

from .. import obj
from .. import bind
from .. import json

from . import enums
from . import utils

########################################################################################################################

## @defgroup nyx_number_message Nyx Number Message
#  @brief Nyx / INDI Number Message.

########################################################################################################################

@utils.nyx_property(
    'name',
    '@name',
    doc = '@brief Gets / sets the name of this property object.'
)
@utils.nyx_property(
    'label',
    '@label',
    doc = '@brief Gets / sets the label of this property object.'
)
@utils.nyx_property(
    'format',
    '@format',
    doc = '@brief Gets / sets the format of this property object.'
)
class NyxNumberProp(json.json_dict.NyxDict):
    """!
    @ingroup nyx_number_message
    @brief Base class for INDI / Nyx number properties.
    """

    pass

########################################################################################################################

class NyxNumberIntProp(NyxNumberProp):
    """!
    @ingroup nyx_number_message
    @brief INDI / Nyx `int32_t` number property.
    """

    ####################################################################################################################

    def __init__(self, name: str, label: str | None, fmt: str, min: int, max: int, step: int, value: int):
        """!
        @brief Allocates a new INDI / Nyx `int32_t` number property.

        @param name Property name.
        @param label Property label.
        @param fmt Printf-style formatting string (%[flags][width]d).
        @param min Range min, ignored if min == max.
        @param max Range max, ignored if min == max.
        @param step Step size, ignored if step == 0.
        @param value Initial `int32_t` value.
        """

        super().__init__(bind.lib.nyx_number_prop_new_int(
            bind.as_bytes(name, allow_none = False),
            bind.as_bytes(label, allow_none = False),
            bind.as_bytes(fmt, allow_none = False),
            min,
            max,
            step,
            value,
        ))

    ####################################################################################################################

    @obj.nyx_callback(bind.nyx_callback_int_t)
    def _nyx_callback_method(self, _vector, _prop, new_value, old_value):

        return all(self._dispatch_callbacks(new_value, old_value))

    ####################################################################################################################

    @property
    def value(self) -> int:
        """!
        @property NyxNumberIntProp::value
        @brief Gets / sets the value of this property object.

        @return The value.
        """

        return bind.lib.nyx_number_prop_get_int(self.ptr)

    ####################################################################################################################

    @value.setter
    def value(self, value: int) -> None:

        bind.lib.nyx_number_prop_set_int(
            self.ptr,
            value,
        )

########################################################################################################################

class NyxNumberUIntProp(NyxNumberProp):
    """!
    @ingroup nyx_number_message
    @brief INDI / Nyx `uint32_t` number property.
    """

    ####################################################################################################################

    def __init__(self, name: str, label: str | None, fmt: str, min: int, max: int, step: int, value: int):
        """!
        @brief Allocates a new INDI / Nyx `uint32_t` number property.

        @param name Property name.
        @param label Property label.
        @param fmt Printf-style formatting string (%[flags][width]{uoxX}).
        @param min Range min, ignored if min == max.
        @param max Range max, ignored if min == max.
        @param step Step size, ignored if step == 0.
        @param value Initial `uint32_t` value.
        """

        super().__init__(bind.lib.nyx_number_prop_new_uint(
            bind.as_bytes(name, allow_none = False),
            bind.as_bytes(label, allow_none = True),
            bind.as_bytes(fmt, allow_none = False),
            min,
            max,
            step,
            value,
        ))

    ####################################################################################################################

    @obj.nyx_callback(bind.nyx_callback_uint_t)
    def _nyx_callback_method(self, _vector, _prop, new_value, old_value):

        return all(self._dispatch_callbacks(new_value, old_value))

    ####################################################################################################################

    @property
    def value(self) -> int:
        """!
        @property NyxNumberUIntProp::value
        @brief Gets the value of this property object.

        @return The value.
        """

        return bind.lib.nyx_number_prop_get_uint(self.ptr)

    ####################################################################################################################

    @value.setter
    def value(self, value: int) -> None:

        bind.lib.nyx_number_prop_set_uint(
            self.ptr,
            value,
        )

########################################################################################################################

class NyxNumberLongProp(NyxNumberProp):
    """!
    @ingroup nyx_number_message
    @brief INDI / Nyx `int64_t` number property.
    """

    ####################################################################################################################

    def __init__(self, name: str, label: str | None, fmt: str, min: int, max: int, step: int, value: int):
        """!
        @brief Allocates a new INDI / Nyx `int64_t` number property.

        @param name Property name.
        @param label Property label.
        @param fmt Printf-style formatting string (%[flags][width]ld).
        @param min Range min, ignored if min == max.
        @param max Range max, ignored if min == max.
        @param step Step size, ignored if step == 0.
        @param value Initial `int64_t` value.
        """

        super().__init__(bind.lib.nyx_number_prop_new_long(
            bind.as_bytes(name, allow_none = False),
            bind.as_bytes(label, allow_none = True),
            bind.as_bytes(fmt, allow_none = False),
            min,
            max,
            step,
            value,
        ))

    ####################################################################################################################

    @obj.nyx_callback(bind.nyx_callback_long_t)
    def _nyx_callback_method(self, _vector, _prop, new_value, old_value):

        return all(self._dispatch_callbacks(new_value, old_value))

    ####################################################################################################################

    @property
    def value(self) -> int:
        """!
        @property NyxNumberLongProp::value
        @brief Gets the value of this property object.

        @return The value.
        """

        return bind.lib.nyx_number_prop_get_long(self.ptr)

    ####################################################################################################################

    @value.setter
    def value(self, value: int) -> None:

        bind.lib.nyx_number_prop_set_long(
            self.ptr,
            value,
        )

########################################################################################################################

class NyxNumberULongProp(NyxNumberProp):
    """!
    @ingroup nyx_number_message
    @brief INDI / Nyx `uint64_t` number property.
    """

    ####################################################################################################################

    def __init__(self, name: str, label: str | None, fmt: str, min: int, max: int, step: int, value: int):
        """!
        @brief Allocates a new INDI / Nyx `uint64_t` number property.

        @param name Property name.
        @param label Property label.
        @param fmt Printf-style formatting string (%[flags][width]l{uoxX}).
        @param min Range min, ignored if min == max.
        @param max Range max, ignored if min == max.
        @param step Step size, ignored if step == 0.
        @param value Initial `uint64_t` value.
        """

        super().__init__(bind.lib.nyx_number_prop_new_ulong(
            bind.as_bytes(name, allow_none = False),
            bind.as_bytes(label, allow_none = True),
            bind.as_bytes(fmt, allow_none = False),
            min,
            max,
            step,
            value,
        ))

    ####################################################################################################################

    @obj.nyx_callback(bind.nyx_callback_ulong_t)
    def _nyx_callback_method(self, _vector, _prop, new_value, old_value):

        return all(self._dispatch_callbacks(new_value, old_value))

    ####################################################################################################################

    @property
    def value(self) -> int:
        """!
        @property NyxNumberULongProp::value
        @brief Gets the value of this property object.

        @return The value.
        """

        return bind.lib.nyx_number_prop_get_ulong(self.ptr)

    ####################################################################################################################

    @value.setter
    def value(self, value: int) -> None:

        bind.lib.nyx_number_prop_set_ulong(
            self.ptr,
            value,
        )

########################################################################################################################

class NyxNumberDoubleProp(NyxNumberProp):
    """!
    @ingroup nyx_number_message
    @brief INDI / Nyx `double` number property.
    """

    ####################################################################################################################

    def __init__(self, name: str, label: str | None, fmt: str, min: float, max: float, step: float, value: float):
        """!
        @brief Allocates a new INDI / Nyx `double` number property.

        @param name Property name.
        @param label Property label.
        @param fmt Printf-style formatting string (%[flags][width]l?{fFeEgGaAm}).
        @param min Range min, ignored if min == max.
        @param max Range max, ignored if min == max.
        @param step Step size, ignored if step == 0.
        @param value Initial `double` value.
        """

        super().__init__(bind.lib.nyx_number_prop_new_double(
            bind.as_bytes(name, allow_none = False),
            bind.as_bytes(label, allow_none = True),
            bind.as_bytes(fmt, allow_none = False),
            min,
            max,
            step,
            value,
        ))

    ####################################################################################################################

    @obj.nyx_callback(bind.nyx_callback_double_t)
    def _nyx_callback_method(self, _vector, _prop, new_value, old_value):

        return all(self._dispatch_callbacks(new_value, old_value))

    ####################################################################################################################

    @property
    def value(self) -> float:
        """!
        @property NyxNumberDoubleProp::value
        @brief Gets the value of this property object.

        @return The value.
        """

        return bind.lib.nyx_number_prop_get_double(self.ptr)

    ####################################################################################################################

    @value.setter
    def value(self, value: float) -> None:

        bind.lib.nyx_number_prop_set_double(
            self.ptr,
            value,
        )

########################################################################################################################

@utils.nyx_property(
    'device',
    '@device',
    doc = '@brief Gets / sets the device of this property object.'
)
@utils.nyx_property(
    'name',
    '@name',
    doc = '@brief Gets / sets the name of this property object.'
)
@utils.nyx_property(
    'state',
    '@state',
    getter = enums.NyxState.nyx_state_int,
    setter = enums.NyxState.nyx_state_str,
    doc = '@brief Gets / sets the state of this property object.'
)
@utils.nyx_property(
    'perm',
    '@perm',
    getter = enums.NyxPerm.nyx_perm_int,
    setter = enums.NyxPerm.nyx_perm_str,
    doc = '@brief Gets / sets the permission of this property object.'
)
class NyxNumberVector(json.json_dict.NyxDict):
    """!
    @ingroup nyx_number_message
    @brief INDI / Nyx number vector.
    """

    ####################################################################################################################

    def __init__(self, device: str, name: str, state: enums.NyxState | int | str, perm: enums.NyxPerm | int | str, props: typing.Iterable[NyxNumberProp], **opts: typing.Any):
        """!
        @brief Allocates a new INDI / Nyx number vector.

        @param device Device name.
        @param name Vector name.
        @param state Vector state.
        @param perm Vector permissions.
        @param props Properties.
        @param opts Options (group, label, hints, timeout, message).
        """

        ################################################################################################################

        super().__init__(bind.lib.nyx_number_vector_new(
            bind.as_bytes(device, allow_none = False),
            bind.as_bytes(name, allow_none = False),
            enums.NyxState.nyx_state_int(state),
            enums.NyxPerm.nyx_perm_int(perm),
            bind.nyx_dict_p(),
            bind.as_opts(opts),
        ))

        ################################################################################################################

        # noinspection PyTypeChecker
        children: json.NyxList = self['children']

        for prop in props:

            if not isinstance(prop, NyxNumberProp):

                raise TypeError('Expected NyxNumberProp')

            children.push(prop)

    ####################################################################################################################

    @obj.nyx_callback(bind.nyx_callback_vector_t)
    def _nyx_callback_method(self, _vector: json.json_dict.NyxDict, modified: bool) -> None:

        self._dispatch_callbacks(bool(modified))

########################################################################################################################

__all__ = [name for name in globals() if name.lower().startswith('nyx')]

########################################################################################################################
