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
from . import json

########################################################################################################################

class NyxNode:

    ####################################################################################################################

    def __init__(
            self,
            node_id: str,
            vectors: typing.List[json.NyxDict],
            indi_url: str | None,
            mqtt_url: str | None,
            nss_url: str | None,
            mqtt_username: str | None,
            mqtt_password: str | None,
            retry_ms: int,
            enable_xml: bool,
    ):

        ################################################################################################################

        # noinspection PyCallingNonCallable
        self._vectors_ptr = (bind.nyx_dict_p * (len(vectors) + 1))()

        ################################################################################################################

        for index, vector in enumerate(vectors):

            if not isinstance(vector, json.NyxDict):

                raise TypeError('Expected Nyx Dict object')

            self._vectors_ptr[index] = ctypes.cast(vector.ptr, bind.nyx_dict_p)

        self._vectors_ptr[-1] = bind.nyx_dict_p()

        ################################################################################################################

        self._ptr = bind.lib.nyx_node_initialize(
            bind.as_bytes(node_id, allow_none = False),
            self._vectors_ptr,
            bind.as_bytes(indi_url, allow_none = True),
            bind.as_bytes(mqtt_url, allow_none = True),
            bind.as_bytes(nss_url, allow_none = True),
            bind.as_bytes(mqtt_username, allow_none = True),
            bind.as_bytes(mqtt_password, allow_none = True),
            bind.nyx_mqtt_handler_t(0),
            retry_ms,
            enable_xml,
        )

    ####################################################################################################################

    @property
    def ptr(self):

        return self._ptr

    ####################################################################################################################

    def finalize(self) -> None:

        bind.lib.nyx_node_finalize(self.ptr, False)

    ####################################################################################################################

    def poll(self, timeout_ms: int) -> None:

        bind.lib.nyx_node_poll(self.ptr, timeout_ms)

    ####################################################################################################################

    def enable(self, device: str, name: str | None = None, message: str | None = None) -> None:

        bind.lib.nyx_node_enable(
            self.ptr,
            bind.as_bytes(device, allow_none = False),
            bind.as_bytes(name, allow_none = True),
            bind.as_bytes(message, allow_none = True)
        )

    ####################################################################################################################

    def disable(self, device: str, name: str | None = None, message: str | None = None) -> None:

        bind.lib.nyx_node_disable(
            self.ptr,
            bind.as_bytes(device, allow_none = False),
            bind.as_bytes(name, allow_none = True),
            bind.as_bytes(message, allow_none = True)
        )

    ####################################################################################################################

    def send_message(self, device: str, message: str | None = None) -> None:

        bind.lib.nyx_node_send_message(
            self.ptr,
            bind.as_bytes(device, allow_none = False),
            bind.as_bytes(message, allow_none = True)
        )

    ####################################################################################################################

    def send_del_property(self, device: str, name: str | None = None, message: str | None = None) -> None:

        bind.lib.nyx_node_send_del_property(
            self.ptr,
            bind.as_bytes(device, allow_none = False),
            bind.as_bytes(name, allow_none = True),
            bind.as_bytes(message, allow_none = True)
        )

    ####################################################################################################################

    def mqtt_sub(self, topic: str, qos: int = 0) -> None:

        bind.lib.nyx_mqtt_sub(
            self.ptr,
            bind.as_bytes(topic, allow_none = False),
            qos
        )

    ####################################################################################################################

    def mqtt_pub(self, topic: str, message: bytes, qos: int = 0) -> None:

        message = bind.as_bytes(message, allow_none = False)

        bind.lib.nyx_mqtt_pub(
            self.ptr,
            bind.as_bytes(topic, allow_none = False),
            len(message),
            message,
            qos
        )

########################################################################################################################

__all__ = ['NyxNode']

########################################################################################################################
