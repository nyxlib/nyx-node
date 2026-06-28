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

                raise TypeError('Expected Nyx Dict')

            self._vectors_ptr[index] = ctypes.cast(vector.ptr, bind.nyx_dict_p)

        self._vectors_ptr[-1] = bind.nyx_dict_p()

        ################################################################################################################

        self._ptr = bind.lib.nyx_node_initialize(
            bind.as_bytes(node_id, allow_none = False),
            self._vectors_ptr,
            bind.as_bytes(indi_url),
            bind.as_bytes(mqtt_url),
            bind.as_bytes(nss_url),
            bind.as_bytes(mqtt_username),
            bind.as_bytes(mqtt_password),
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

    def notify(self, object: json.NyxDict | None = None) -> bool:

        return bool(bind.lib.nyx_node_notify(self.ptr, object.ptr)) if object is not None else False

    ####################################################################################################################

    def enable(self, device: str, name: str | None = None, message: str | None = None) -> None:

        bind.lib.nyx_node_enable(self.ptr, bind.as_bytes(device, allow_none = False), bind.as_bytes(name), bind.as_bytes(message))

    ####################################################################################################################

    def disable(self, device: str, name: str | None = None, message: str | None = None) -> None:

        bind.lib.nyx_node_disable(self.ptr, bind.as_bytes(device, allow_none = False), bind.as_bytes(name), bind.as_bytes(message))

    ####################################################################################################################

    def send_message(self, device: str, message: str | None = None) -> None:

        bind.lib.nyx_node_send_message(self.ptr, bind.as_bytes(device, allow_none = False), bind.as_bytes(message))

    ####################################################################################################################

    def send_del_property(self, device: str, name: str | None = None, message: str | None = None) -> None:

        bind.lib.nyx_node_send_del_property(self.ptr, bind.as_bytes(device, allow_none = False), bind.as_bytes(name), bind.as_bytes(message))

########################################################################################################################

__all__ = ['NyxNode']

########################################################################################################################
