/* NyxNode
 * Author: Jérôme ODIER <jerome.odier@lpsc.in2p3.fr>
 * SPDX-License-Identifier: GPL-2.0-only (Mongoose backend) or GPL-3.0+
 */

/*--------------------------------------------------------------------------------------------------------------------*/

#ifndef NYX_NODE_HPP
#define NYX_NODE_HPP

/*--------------------------------------------------------------------------------------------------------------------*/

#include "json/json_dict.hpp"
#include "indi/enums.hpp"

/*--------------------------------------------------------------------------------------------------------------------*/

namespace nyx {

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * @defgroup NODE_CPP Nyx node (C++)
 * @brief Nyx node C++ API.
 */

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * @ingroup NODE_CPP
 * @brief MQTT event type.
 */
using MQTTEvent = nyx_event_type_t;

/* NODE                                                                                                               */
/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * @brief Non-owning Nyx node reference passed to C++ callbacks.
 */
class NodeRef
{
public:
    /*----------------------------------------------------------------------------------------------------------------*/

    explicit NodeRef(const nyx_node_t *ptr) : m_ptr(ptr)
    {
        check_ptr(m_ptr, "Invalid nyx_node_t pointer");
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    const nyx_node_t *ptr() const
    {
        check_ptr(m_ptr, "Nyx node has been closed");

        return m_ptr;
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    /**
     * @brief Enables a device or a vector and notifies clients.
     *
     * @param device Device name.
     * @param name Optional vector name, where nullptr means the whole device.
     * @param message Optional human-oriented message.
     */
    void enable(STR_t device, STR_t name = nullptr, STR_t message = nullptr) const
    {
        nyx_node_enable(ptr(), device, name, message);
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    /**
     * @brief Disables a device or a vector and notifies clients.
     *
     * @param device Device name.
     * @param name Optional vector name, where nullptr means the whole device.
     * @param message Optional human-oriented message.
     */
    void disable(STR_t device, STR_t name = nullptr, STR_t message = nullptr) const
    {
        nyx_node_disable(ptr(), device, name, message);
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    /**
     * @brief Sends a human-oriented message to the clients.
     *
     * @param device Device name.
     * @param message Human-oriented message.
     */
    void send_message(STR_t device, STR_t message = nullptr) const
    {
        nyx_node_send_message(ptr(), device, message);
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    /**
     * @brief Sends a @c del-property message to the clients.
     *
     * @param device Device name.
     * @param name Optional vector name, where nullptr means the whole device.
     * @param message Optional human-oriented message.
     */
    void send_del_property(STR_t device, STR_t name = nullptr, STR_t message = nullptr) const
    {
        nyx_node_send_del_property(ptr(), device, name, message);
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    /**
     * @brief If MQTT is enabled, subscribes to an MQTT topic.
     *
     * @param topic MQTT topic.
     * @param qos MQTT Quality Of Service.
     */
    void mqtt_sub(STR_t topic, int qos = 0) const
    {
        nyx_mqtt_sub(ptr(), topic, qos);
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    /**
     * @brief If MQTT is enabled, publishes an MQTT message.
     *
     * @param topic MQTT topic.
     * @param message_size Number of message payload bytes.
     * @param message_buff Message payload.
     * @param qos MQTT Quality Of Service.
     * @note The message payload may contain arbitrary binary data.
     */
    void mqtt_pub(STR_t topic, size_t message_size, BUFF_t message_buff, int qos = 0) const
    {
        nyx_mqtt_pub(ptr(), topic, message_size, message_buff, qos);
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    /**
     * @brief If Nyx Stream is enabled, publishes an entry to a stream.
     *
     * @param device Device name.
     * @param stream Stream name.
     * @param n_fields Number of fields.
     * @param field_hashes Field hashes, one per field.
     * @param field_sizes Field payload sizes, one per field.
     * @param field_buffs Field payload buffers, one per field.
     * @note Field payloads may contain arbitrary binary data.
     */
    void nss_pub(STR_t device, STR_t stream, size_t n_fields, const uint32_t field_hashes[], const size_t field_sizes[], const buff_t field_buffs[]) const
    {
        nyx_nss_pub(ptr(), device, stream, n_fields, field_hashes, field_sizes, field_buffs);
    }

    /*----------------------------------------------------------------------------------------------------------------*/

private:
    /*----------------------------------------------------------------------------------------------------------------*/

    const nyx_node_t *m_ptr = nullptr;

    /*----------------------------------------------------------------------------------------------------------------*/
};

/*--------------------------------------------------------------------------------------------------------------------*/

template<void (*Callback)(const NodeRef &, MQTTEvent, size_t, BUFF_t, size_t, BUFF_t)>
inline void mqtt_callback_dispatch(
    const nyx_node_t *node,
    MQTTEvent event_type,
    size_t topic_size,
    BUFF_t topic_buff,
    size_t message_size,
    BUFF_t message_buff
) {
    NodeRef node_obj(node);

    Callback(node_obj, event_type, topic_size, topic_buff, message_size, message_buff);
}

/*--------------------------------------------------------------------------------------------------------------------*/

template<void (*Callback)(const NodeRef &)>
inline void mqtt_open_callback_dispatch(
    const nyx_node_t *node,
    MQTTEvent event_type,
    size_t,
    BUFF_t,
    size_t,
    BUFF_t
) {
    if(event_type == NYX_NODE_EVENT_OPEN)
    {
        NodeRef node_obj(node);

        Callback(node_obj);
    }
}

/*--------------------------------------------------------------------------------------------------------------------*/

template<void (*Callback)(const NodeRef &, size_t, BUFF_t, size_t, BUFF_t)>
inline void mqtt_msg_callback_dispatch(
    const nyx_node_t *node,
    MQTTEvent event_type,
    size_t topic_size,
    BUFF_t topic_buff,
    size_t message_size,
    BUFF_t message_buff
) {
    if(event_type == NYX_NODE_EVENT_MSG)
    {
        NodeRef node_obj(node);

        Callback(node_obj, topic_size, topic_buff, message_size, message_buff);
    }
}

/*--------------------------------------------------------------------------------------------------------------------*/

template<void (*Callback)()>
inline void timer_callback_dispatch(void *)
{
    Callback();
}

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * @brief Nyx node construction options.
 */
struct NodeOpts
{
    STR_t indi_url = nullptr;
    STR_t mqtt_url = nullptr;
    STR_t nss_url = nullptr;

    STR_t mqtt_username = nullptr;
    STR_t mqtt_password = nullptr;

    nyx_mqtt_handler_t mqtt_handler = nullptr;

    uint32_t retry_ms = 3000;
    bool enable_xml = true;

    /*----------------------------------------------------------------------------------------------------------------*/

    /**
     * @brief Registers an MQTT event handler.
     *
     * @note The callback registration is stored in the node options before construction.
     */
    template<void (*Callback)(const NodeRef &, MQTTEvent, size_t, BUFF_t, size_t, BUFF_t)>
    void on_mqtt()
    {
        mqtt_handler = mqtt_callback_dispatch<Callback>;
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    /**
     * @brief Registers an MQTT open event handler.
     *
     * @note The callback registration is stored in the node options before construction.
     */
    template<void (*Callback)(const NodeRef &)>
    void on_mqtt()
    {
        mqtt_handler = mqtt_open_callback_dispatch<Callback>;
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    /**
     * @brief Registers an MQTT message event handler.
     *
     * @note The callback registration is stored in the node options before construction.
     */
    template<void (*Callback)(const NodeRef &, size_t, BUFF_t, size_t, BUFF_t)>
    void on_mqtt()
    {
        mqtt_handler = mqtt_msg_callback_dispatch<Callback>;
    }
};

/*--------------------------------------------------------------------------------------------------------------------*/

/**
 * @ingroup NODE_CPP
 * @brief Nyx node exposing INDI, MQTT and Nyx Stream endpoints.
 */
class Node
{
public:
    /*----------------------------------------------------------------------------------------------------------------*/

    /**
     * @brief Allocates and initializes a Nyx node.
     *
     * @param node_id Unique node identifier.
     * @param vectors Array of vectors.
     * @param opts Node options.
     */
    Node(
        STR_t node_id,
        const std::vector<Dict> &vectors,
        const NodeOpts &opts = NodeOpts()
    ) : m_vectors(vectors)
    {
        initialize(node_id, opts);
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    Node(
        STR_t node_id,
        std::initializer_list<Dict> vectors,
        const NodeOpts &opts = NodeOpts()
    ) : Node(
        node_id,
        std::vector<Dict>(vectors),
        opts
    )
    {
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    ~Node()
    {
        close();
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    Node(const Node &) = delete;
    Node &operator=(const Node &) = delete;

    /*----------------------------------------------------------------------------------------------------------------*/

    nyx_node_t *ptr() const
    {
        check_ptr(m_ptr, "Nyx node has been closed");

        return m_ptr;
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    bool valid() const
    {
        return m_ptr != nullptr;
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    /**
     * @brief Finalizes the Nyx node.
     */
    void close()
    {
        if(m_ptr != nullptr)
        {
            nyx_node_finalize(m_ptr, false);

            m_ptr = nullptr;
        }
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    /**
     * @brief Adds a new timer.
     *
     * @param interval_ms Interval [milliseconds].
     * @param callback Callback to be invoked.
     * @note Timers are triggered by poll().
     */
    template<typename Callback>
    void add_timer(uint32_t interval_ms, Callback callback) const
    {
        nyx_node_add_timer(ptr(), interval_ms, callback, nullptr);
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    /**
     * @brief Registers a timer handler.
     *
     * @param interval_ms Interval [milliseconds].
     * @param callback Timer handler.
     * @note Timers are triggered by poll().
     */
    template<typename Callback>
    void on_timer(uint32_t interval_ms, Callback callback) const
    {
        add_timer(interval_ms, callback);
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    /**
     * @brief Registers a timer handler.
     *
     * @param interval_ms Interval [milliseconds].
     * @note Timers are triggered by poll().
     */
    template<void (*Callback)()>
    void on_timer(uint32_t interval_ms) const
    {
        nyx_node_add_timer(ptr(), interval_ms, timer_callback_dispatch<Callback>, nullptr);
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    /**
     * @brief Adds a new timer.
     *
     * @param interval_ms Interval [milliseconds].
     * @param callback Callback to be invoked.
     * @param arg Callback argument.
     * @note Timers are triggered by poll().
     */
    template<typename Callback, typename Arg>
    void add_timer(uint32_t interval_ms, Callback callback, Arg *arg) const
    {
        nyx_node_add_timer(ptr(), interval_ms, callback, arg);
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    /**
     * @brief Registers a timer handler.
     *
     * @param interval_ms Interval [milliseconds].
     * @param callback Timer handler.
     * @param arg Callback argument.
     * @note Timers are triggered by poll().
     */
    template<typename Callback, typename Arg>
    void on_timer(uint32_t interval_ms, Callback callback, Arg *arg) const
    {
        add_timer(interval_ms, callback, arg);
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    /**
     * @brief Performs a single poll iteration.
     *
     * @param timeout_ms Timeout [milliseconds].
     * @note @c timeout_ms determines the minimum timer resolution.
     */
    void poll(uint32_t timeout_ms) const
    {
        nyx_node_poll(ptr(), timeout_ms);
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    /**
     * @brief Enables a device or a vector and notifies clients.
     *
     * @param device Device name.
     * @param name Optional vector name, where nullptr means the whole device.
     * @param message Optional human-oriented message.
     */
    void enable(STR_t device, STR_t name = nullptr, STR_t message = nullptr) const
    {
        nyx_node_enable(ptr(), device, name, message);
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    /**
     * @brief Disables a device or a vector and notifies clients.
     *
     * @param device Device name.
     * @param name Optional vector name, where nullptr means the whole device.
     * @param message Optional human-oriented message.
     */
    void disable(STR_t device, STR_t name = nullptr, STR_t message = nullptr) const
    {
        nyx_node_disable(ptr(), device, name, message);
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    /**
     * @brief Sends a human-oriented message to the clients.
     *
     * @param device Device name.
     * @param message Human-oriented message.
     */
    void send_message(STR_t device, STR_t message = nullptr) const
    {
        nyx_node_send_message(ptr(), device, message);
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    /**
     * @brief Sends a @c del-property message to the clients.
     *
     * @param device Device name.
     * @param name Optional vector name, where nullptr means the whole device.
     * @param message Optional human-oriented message.
     */
    void send_del_property(STR_t device, STR_t name = nullptr, STR_t message = nullptr) const
    {
        nyx_node_send_del_property(ptr(), device, name, message);
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    /**
     * @brief If MQTT is enabled, subscribes to an MQTT topic.
     *
     * @param topic MQTT topic.
     * @param qos MQTT Quality Of Service.
     */
    void mqtt_sub(STR_t topic, int qos = 0) const
    {
        nyx_mqtt_sub(ptr(), topic, qos);
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    /**
     * @brief If MQTT is enabled, publishes an MQTT message.
     *
     * @param topic MQTT topic.
     * @param message_size Number of message payload bytes.
     * @param message_buff Message payload.
     * @param qos MQTT Quality Of Service.
     * @note The message payload may contain arbitrary binary data.
     */
    void mqtt_pub(STR_t topic, size_t message_size, BUFF_t message_buff, int qos = 0) const
    {
        nyx_mqtt_pub(ptr(), topic, message_size, message_buff, qos);
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    /**
     * @brief If Nyx Stream is enabled, publishes an entry to a stream.
     *
     * @param device Device name.
     * @param stream Stream name.
     * @param n_fields Number of fields.
     * @param field_hashes Field hashes, one per field.
     * @param field_sizes Field payload sizes, one per field.
     * @param field_buffs Field payload buffers, one per field.
     * @note Field payloads may contain arbitrary binary data.
     */
    void nss_pub(STR_t device, STR_t stream, size_t n_fields, const uint32_t field_hashes[], const size_t field_sizes[], const buff_t field_buffs[]) const
    {
        nyx_nss_pub(ptr(), device, stream, n_fields, field_hashes, field_sizes, field_buffs);
    }

    /*----------------------------------------------------------------------------------------------------------------*/

private:
    /*----------------------------------------------------------------------------------------------------------------*/

    void initialize(
        STR_t node_id,
        const NodeOpts &opts
    ) {
        for(const auto &vector: m_vectors)
        {
            m_vector_ptrs.push_back(vector.dict_ptr());
        }

        m_vector_ptrs.push_back(nullptr);

        m_ptr = nyx_node_initialize(
            node_id,
            m_vector_ptrs.data(),
            opts.indi_url,
            opts.mqtt_url,
            opts.nss_url,
            opts.mqtt_username,
            opts.mqtt_password,
            opts.mqtt_handler,
            opts.retry_ms,
            opts.enable_xml
        );

        check_ptr(m_ptr, "Invalid nyx_node_t pointer");
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    nyx_node_t *m_ptr = nullptr;

    std::vector<Dict> m_vectors;
    std::vector<nyx_dict_t *> m_vector_ptrs;

    /*----------------------------------------------------------------------------------------------------------------*/
};

/*--------------------------------------------------------------------------------------------------------------------*/

} /* namespace nyx */

/*--------------------------------------------------------------------------------------------------------------------*/

#endif /* NYX_NODE_HPP */

/*--------------------------------------------------------------------------------------------------------------------*/
