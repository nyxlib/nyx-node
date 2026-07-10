# -*- coding: utf-8 -*-
########################################################################################################################
# NyxNode
# Author: Jérôme ODIER <jerome.odier@lpsc.in2p3.fr>
# SPDX-License-Identifier: GPL-3.0+
########################################################################################################################

import enum
import ctypes
import typing

########################################################################################################################

from . import bind
from . import json

########################################################################################################################

## @defgroup NODE_PY Nyx node
#  @brief Nyx node.

########################################################################################################################

class NyxMQTTEvent(enum.IntEnum):
    """!
    @ingroup NODE_PY
    @brief MQTT event type.

    | Symbol              | Value | Description             |
    | :------------------ | :---- | :---------------------- |
    | `NyxMQTTEvent.OPEN` | 1100  | A connection is opened. |
    | `NyxMQTTEvent.MSG`  | 1101  | A message is received.  |
    """

    OPEN = 1100
    MSG = 1101

########################################################################################################################

class NyxNode:
    """!
    @ingroup NODE_PY
    @brief Nyx node exposing INDI, MQTT and Nyx Stream endpoints.
    """

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
        """!
        @brief Allocates and initializes a Nyx node.

        @param node_id Unique node identifier.
        @param vectors Array of vectors.
        @param indi_url Optional INDI URL.
        @param mqtt_url Optional MQTT URL.
        @param nss_url Optional Nyx Stream URL.
        @param mqtt_username Optional MQTT username.
        @param mqtt_password Optional MQTT password.
        @param retry_ms Connect retry time [milliseconds].
        @param enable_xml Enables the XML messages for INDI compatibility.
        """

        ################################################################################################################

        self._vectors = tuple(vectors)

        ################################################################################################################

        self._mqtt_open_handlers = []
        self._mqtt_msg_handlers = []
        self._timer_contexts = []

        ################################################################################################################

        self._mqtt_callback = bind.nyx_mqtt_handler_t(self._on_mqtt)

        ################################################################################################################

        # noinspection PyCallingNonCallable
        self._vectors_ptr = (bind.nyx_dict_p * (len(self._vectors) + 1))()

        ################################################################################################################

        for i, vector in enumerate(self._vectors):

            if not isinstance(vector, json.NyxDict):

                raise TypeError('Expected Nyx Dict object')

            self._vectors_ptr[i] = ctypes.cast(vector.ptr, bind.nyx_dict_p)

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
            self._mqtt_callback,
            retry_ms,
            enable_xml,
        )

    ####################################################################################################################

    @property
    def ptr(self):
        """!
        @private
        @brief C pointer to the Nyx node.

        @return The Nyx node pointer.
        """

        return self._ptr

    ####################################################################################################################

    def _on_mqtt(
        self,
        _,
        event_type: int,
        topic_size: int,
        topic_buff: bind.c_void_p,
        message_size: int,
        message_buff: bind.c_void_p,
    ) -> None:

        ################################################################################################################

        event_type = NyxMQTTEvent(event_type)

        ################################################################################################################
        # OPEN EVENT                                                                                                   #
        ################################################################################################################

        if event_type == NyxMQTTEvent.OPEN:

            callbacks = tuple(self._mqtt_open_handlers)

            for callback in callbacks:

                callback()

        ################################################################################################################
        # MSG EVENT                                                                                                    #
        ################################################################################################################

        elif event_type == NyxMQTTEvent.MSG:

            callbacks = tuple(self._mqtt_msg_handlers)

            if callbacks:

                topic = ctypes.string_at(topic_buff, topic_size) if topic_size else b''
                message = ctypes.string_at(message_buff, message_size) if message_size else b''

                topic = topic.decode('utf-8')

                for callback in callbacks:

                    callback(topic, message)

    ####################################################################################################################

    @staticmethod
    @bind.nyx_timer_callback_t
    def _on_timer(arg: bind.c_void_p) -> None:

        ctypes.cast(arg, ctypes.POINTER(ctypes.py_object)).contents.value()

    ####################################################################################################################

    def on_mqtt(self, event_type: NyxMQTTEvent) -> typing.Callable:
        """!
        @brief Registers an MQTT event handler.

        @param event_type MQTT event type.
        @return A decorator registering the MQTT event handler.

        @code{.py}
        with nyx.NyxNode(...) as node:

            @node.on_mqtt(nyx.NyxMQTTEvent.OPEN)
            def on_mqtt_open():
                ...

            @node.on_mqtt(nyx.NyxMQTTEvent.MSG)
            def on_mqtt_msg(topic, message):
                ...
        @endcode
        """

        ################################################################################################################

        if not isinstance(event_type, NyxMQTTEvent):

            raise TypeError('Expected NyxMQTTEvent enum')

        ################################################################################################################

        def decorate(callback: typing.Callable) -> typing.Callable:

            if not callable(callback):

                raise TypeError('Expected a callable MQTT handler')

            ############################################################################################################

            if   event_type == NyxMQTTEvent.OPEN:

                self._mqtt_open_handlers.append(callback)

            elif event_type == NyxMQTTEvent.MSG:

                self._mqtt_msg_handlers.append(callback)

            else:

                raise ValueError(f'Unsupported MQTT event: {event_type!r}')

            ############################################################################################################

            return callback

        ################################################################################################################

        return decorate

    ####################################################################################################################

    def on_timer(self, interval_ms: int):
        """!
        @brief Registers a timer handler.

        @param interval_ms Interval [milliseconds].
        @return A decorator registering the timer handler.

        @note Timers are triggered by the @ref nyx.node.NyxNode.poll method.

        @code{.py}
        with nyx.NyxNode(...) as node:

            @node.on_timer(50)
            def on_timer():
                ...
        @endcode
        """

        ################################################################################################################

        if not isinstance(interval_ms, int):

            raise TypeError('Expected timer interval in milliseconds')

        if interval_ms < 1:

            raise ValueError('Timer interval must be positive')

        ################################################################################################################

        def decorate(callback: typing.Callable) -> typing.Callable:

            if not callable(callback):

                raise TypeError('Expected a callable timer handler')

            ############################################################################################################

            timer_context = ctypes.py_object(callback)

            self._timer_contexts.append(timer_context)

            ############################################################################################################

            bind.lib.nyx_node_add_timer(
                self.ptr,
                interval_ms,
                type(self)._on_timer,
                bind.c_void_p(ctypes.addressof(timer_context)),
            )

            ############################################################################################################

            return callback

        ################################################################################################################

        return decorate

    ####################################################################################################################

    def close(self) -> None:
        """!
        @brief Finalizes the Nyx node.

        @return None
        """

        bind.lib.nyx_node_finalize(self.ptr, False)

    ####################################################################################################################

    def poll(self, timeout_ms: int) -> None:
        """! @brief Performs a single poll iteration.

        @param timeout_ms Timeout [milliseconds].
        @return None

        @note \c timeout_ms determines the minimum timer resolution.
        """

        bind.lib.nyx_node_poll(self.ptr, timeout_ms)

    ####################################################################################################################

    def enable(self, device: str, name: str | None = None, message: str | None = None) -> None:
        """!
        @brief Enables a device or a vector and notifies clients.

        @param device Device name.
        @param name Optional vector name (`None` means whole device).
        @param message Optional human-oriented message.
        @return None
        """

        bind.lib.nyx_node_enable(
            self.ptr,
            bind.as_bytes(device, allow_none = False),
            bind.as_bytes(name, allow_none = True),
            bind.as_bytes(message, allow_none = True)
        )

    ####################################################################################################################

    def disable(self, device: str, name: str | None = None, message: str | None = None) -> None:
        """!
        @brief Disables a device or a vector and notifies clients.

        @param device Device name.
        @param name Optional vector name (`None` means the whole device).
        @param message Optional human-oriented message.
        @return None
        """

        bind.lib.nyx_node_disable(
            self.ptr,
            bind.as_bytes(device, allow_none = False),
            bind.as_bytes(name, allow_none = True),
            bind.as_bytes(message, allow_none = True)
        )

    ####################################################################################################################

    def send_message(self, device: str, message: str | None = None) -> None:
        """!
        @brief Sends a human-oriented message to the clients.

        @param device Device name.
        @param message Human-oriented message.
        @return None
        """

        bind.lib.nyx_node_send_message(
            self.ptr,
            bind.as_bytes(device, allow_none = False),
            bind.as_bytes(message, allow_none = True)
        )

    ####################################################################################################################

    def send_del_property(self, device: str, name: str | None = None, message: str | None = None) -> None:
        """!
        @brief Sends a `del-property` message to the clients.

        @param device Device name.
        @param name Optional vector name (`None` means the whole device).
        @param message Optional human-oriented message.
        @return None
        """

        bind.lib.nyx_node_send_del_property(
            self.ptr,
            bind.as_bytes(device, allow_none = False),
            bind.as_bytes(name, allow_none = True),
            bind.as_bytes(message, allow_none = True)
        )

    ####################################################################################################################

    def mqtt_sub(self, topic: str, qos: int = 0) -> None:
        """!
        @brief If MQTT is enabled, subscribes to an MQTT topic.

        @param topic MQTT topic.
        @param qos MQTT Quality Of Service.
        @return None

        @note MQTT handlers have to be added with the @ref nyx.node.NyxNode.on_mqtt decorator.
        """

        bind.lib.nyx_mqtt_sub(
            self.ptr,
            bind.as_bytes(topic, allow_none = False),
            qos
        )

    ####################################################################################################################

    def mqtt_pub(self, topic: str, message: bytes, qos: int = 0) -> None:
        """!
        @brief If MQTT is enabled, publishes an MQTT message.

        @param topic MQTT topic.
        @param message Message payload.
        @param qos MQTT Quality Of Service.
        @return None

        @note The message payload may contain arbitrary binary data.
        """

        message = bind.as_bytes(message, allow_none = False)

        bind.lib.nyx_mqtt_pub(
            self.ptr,
            bind.as_bytes(topic, allow_none = False),
            len(message),
            message,
            qos
        )

    ####################################################################################################################

    def __enter__(self):

        return self

    ####################################################################################################################

    def __exit__(self, exc_type, exc_value, traceback):

        self.close()

########################################################################################################################

__all__ = ['NyxMQTTEvent', 'NyxNode']

########################################################################################################################
