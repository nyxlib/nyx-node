# -*- coding: utf-8 -*-
########################################################################################################################
# NyxNode
# Author: Jérôme ODIER <jerome.odier@lpsc.in2p3.fr>
# SPDX-License-Identifier: GPL-3.0+
########################################################################################################################

import typing

########################################################################################################################

def nyx_property(name: str, key: str, kind: typing.Type = str, getter: typing.Callable[[typing.Any], typing.Any] | None = None, setter: typing.Callable | None = None) -> typing.Callable:

    def decorate(cls: type) -> type:

        ################################################################################################################

        if name in cls.__dict__:

            raise AttributeError(f'Nyx property already exists: {name}')

        ################################################################################################################

        def get_value(self) -> typing.Any:
            f"""
            @brief Gets the @c {name} attribute of this object (type: @c {kind.__name__}).'
                    
            @return The current value.
            """

            value = self[key].value

            if getter is not None:

                value = getter(value)

            return kind(value)

        ################################################################################################################

        def set_value(self, value: typing.Any) -> None:
            f"""
            @brief Sets the @c {name} attribute of this object (type: @c {kind.__name__}).'

            @return @c True if the value was modified, @c False otherwise.
            """

            if setter is not None:

                value = setter(value)

            self[key].value = value

        ################################################################################################################

        setattr(cls, name, property(get_value, set_value))

        ################################################################################################################

        return cls

    return decorate

########################################################################################################################

__all__ = [name for name in globals() if name.lower().startswith('nyx')]

########################################################################################################################
