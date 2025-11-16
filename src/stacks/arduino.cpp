/* NyxNode
 * Author: Jérôme ODIER <jerome.odier@lpsc.in2p3.fr>
 * SPDX-License-Identifier: GPL-2.0-only (Mongoose backend) or GPL-3.0+
 */

/*--------------------------------------------------------------------------------------------------------------------*/
#if defined(ARDUINO)
/*--------------------------------------------------------------------------------------------------------------------*/

#include <Arduino.h>
#include <PubSubClient.h>
#include <arduino-timer.h>

#if defined(NYX_HAS_WIFI)
#  if defined(ARDUINO_ARCH_ESP8266)
#    include <ESP8266WiFi.h>
#  else
#    include <WiFi.h>
#  endif
#elif defined(NYX_HAS_ETHERNET)
#  include <Ethernet.h>

#  include <Dns.h>
#else
#  error "Neither NYX_HAS_WIFI nor NYX_HAS_ETHERNET is defined!"
#endif

#include "../nyx_node_internal.h"

/*--------------------------------------------------------------------------------------------------------------------*/

#if defined(NYX_HAS_ETHERNET)
static DNSClient EthDNS;
#endif

/*--------------------------------------------------------------------------------------------------------------------*/

static nyx_node_t *nyx_node = nullptr;

/*--------------------------------------------------------------------------------------------------------------------*/

struct nyx_stack_s
{
    /*----------------------------------------------------------------------------------------------------------------*/

    Timer<> timer = timer_create_default();

    /*----------------------------------------------------------------------------------------------------------------*/

    #ifdef NYX_HAS_WIFI
    WiFiClient tcp_client;
    WiFiClient redis_client;
    #endif

    #ifdef NYX_HAS_ETHERNET
    EthernetClient tcp_client;
    EthernetClient redis_client;
    #endif

    /*----------------------------------------------------------------------------------------------------------------*/

    PubSubClient mqtt_client;

    /*----------------------------------------------------------------------------------------------------------------*/

    IPAddress redis_ip;
    int redis_port = 6379;

    /*----------------------------------------------------------------------------------------------------------------*/

    nyx_stack_s(): mqtt_client(tcp_client) {}

    /*----------------------------------------------------------------------------------------------------------------*/
};

/*--------------------------------------------------------------------------------------------------------------------*/
/* LOGGER                                                                                                             */
/*--------------------------------------------------------------------------------------------------------------------*/

void nyx_log(nyx_log_level_t level, STR_t file, STR_t func, int line, STR_t fmt, ...)
{
    if(level <= nyx_log_level)
    {
        /*------------------------------------------------------------------------------------------------------------*/

        STR_t p;

        p = strrchr(file, '/');

        if(p != nullptr)
        {
            file = p + 1;
        }
        else
        {
            p = strrchr(file, '\\');

            if(p != nullptr)
            {
                file = p + 1;
            }
        }

        /*------------------------------------------------------------------------------------------------------------*/

        fprintf(stdout, "%s - %s:%d %s() - ", nyx_log_level_to_str(level), file, line, func);

        /*------------------------------------------------------------------------------------------------------------*/

        va_list ap;
        va_start(ap, fmt);
        vfprintf(stdout, fmt, ap);
        va_end(ap);

        /*------------------------------------------------------------------------------------------------------------*/

        fputc('\r', stdout);
        fputc('\n', stdout);

        /*------------------------------------------------------------------------------------------------------------*/

        fflush(stdout);

        /*------------------------------------------------------------------------------------------------------------*/
    }

    if(level == NYX_LOG_LEVEL_FATAL)
    {
        #if defined(ARDUINO_ARCH_ESP32) || defined(ARDUINO_ARCH_ESP8266)
        ESP.restart();
        #endif

        for(;;)
        {
            delay(1);
        }
    }
}

/*--------------------------------------------------------------------------------------------------------------------*/
/* TCP & MQTT                                                                                                         */
/*--------------------------------------------------------------------------------------------------------------------*/

void internal_mqtt_sub(nyx_node_t *node, nyx_str_t topic, __NYX_UNUSED__ int qos)
{
    auto stack = node->stack;

    if(stack->mqtt_client.connected())
    {
        if(!stack->mqtt_client.subscribe(topic.buf, 0))
        {
            NYX_LOG_ERROR("Cannot subscribe to %s", topic.buf);
        }
    }
}

/*--------------------------------------------------------------------------------------------------------------------*/

void internal_mqtt_pub(nyx_node_t *node, nyx_str_t topic, nyx_str_t message, __NYX_UNUSED__ int qos)
{
    auto stack = node->stack;

    if(stack->mqtt_client.connected())
    {
        if(!stack->mqtt_client.publish(
            topic.buf,
            reinterpret_cast<uint8_t *>(message.buf),
            reinterpret_cast<unsigned int>(message.len)
        )) {
            NYX_LOG_ERROR("Cannot publish to %s", topic.buf);
        }
    }
}

/*--------------------------------------------------------------------------------------------------------------------*/

void internal_redis_pub(nyx_node_t *node, nyx_str_t message)
{
    auto stack = node->stack;

    if(stack->redis_client.connected())
    {
        stack->redis_client.write(message.buf, message.len);
    }
}

/*--------------------------------------------------------------------------------------------------------------------*/
/* STACK                                                                                                              */
/*--------------------------------------------------------------------------------------------------------------------*/

static bool _parse_host_port(String url, IPAddress &ip, int &port, int default_port)
{
    /*----------------------------------------------------------------------------------------------------------------*/
    /* SKIP PROTOCOL                                                                                                  */
    /*----------------------------------------------------------------------------------------------------------------*/

    int proto_sep = url.indexOf("://");

    if(proto_sep >= 0)
    {
        url = url.substring(proto_sep + 3);
    }

    /*----------------------------------------------------------------------------------------------------------------*/
    /* GET HOST AND PORT                                                                                              */
    /*----------------------------------------------------------------------------------------------------------------*/

    String host;

    int colon = url.indexOf(':');

    if(colon >= 0)
    {
        host = url.substring(0, colon);
        port = strtol(url.substring(colon + 1).c_str(), nullptr, 10);
    }
    else
    {
        host = url;
        port = default_port;
    }

    /*----------------------------------------------------------------------------------------------------------------*/
    /* RESOLVE DNS DOMAIN                                                                                             */
    /*----------------------------------------------------------------------------------------------------------------*/

    #ifdef NYX_HAS_WIFI
    return ip.fromString(host.c_str()) || WiFi.hostByName(host.c_str(), ip) == 1;
    #endif

    #ifdef NYX_HAS_ETHERNET
    return ip.fromString(host.c_str()) || EthDNS.getHostByName(host.c_str(), ip) == 1;
    #endif

    /*----------------------------------------------------------------------------------------------------------------*/
}

/*--------------------------------------------------------------------------------------------------------------------*/

static void _mqtt_callback(char *topic, uint8_t *buff, unsigned int size)
{
    nyx_str_t message = {
        reinterpret_cast<str_t>(buff),
        reinterpret_cast<size_t>(size),
    };

    nyx_node->mqtt_handler(nyx_node, NYX_NODE_EVENT_MSG, nyx_str_s(topic), message);
}

/*--------------------------------------------------------------------------------------------------------------------*/

static void _retry_timer_handler(void *arg)
{
    auto node = static_cast<nyx_node_t *>(arg);

    auto stack = static_cast<nyx_stack_t *>(node->stack);

    /*----------------------------------------------------------------------------------------------------------------*/
    /* MQTT                                                                                                           */
    /*----------------------------------------------------------------------------------------------------------------*/

    if(node->mqtt_url != nullptr && node->mqtt_url[0] != '\0' && !stack->mqtt_client.connected())
    {
        if(stack->mqtt_client.connect(node->node_id.buf, node->mqtt_username, node->mqtt_password))
        {
            NYX_LOG_INFO("MQTT support is enabled");

            node->mqtt_handler(
                node,
                NYX_NODE_EVENT_OPEN,
                node->node_id,
                node->node_id
            );
        }
    }

    /*----------------------------------------------------------------------------------------------------------------*/
    /* REDIS                                                                                                          */
    /*----------------------------------------------------------------------------------------------------------------*/

    if(node->redis_url != nullptr && node->redis_url[0] != '\0' && !stack->redis_client.connected())
    {
        if(stack->redis_client.connect(stack->redis_ip, stack->redis_port))
        {
            NYX_LOG_INFO("Redis support is enabled");

            nyx_redis_auth(
                node,
                node->redis_username,
                node->redis_password
            );
        }
    }

    /*----------------------------------------------------------------------------------------------------------------*/
}

/*--------------------------------------------------------------------------------------------------------------------*/

static void _ping_timer_handler(void *arg)
{
    nyx_node_ping(static_cast<nyx_node_t *>(arg));
}

/*--------------------------------------------------------------------------------------------------------------------*/

static uint16_t _mqtt_buffer_size()
{
    /*----------------------------------------------------------------------------------------------------------------*/

    #if defined(ARDUINO_ARCH_ESP32) || defined(ARDUINO_ARCH_ESP8266)

    uint32_t free_heap = ESP.getFreeHeap();

    #elif defined(NYX_RAM_SIZE)

    uint32_t free_heap = NYX_RAM_SIZE;

    #else
    #  error "NYX_RAM_SIZE is not defined!"
    #endif

    /*----------------------------------------------------------------------------------------------------------------*/

    uint16_t buff_size;

    /**/ if(free_heap > 2 * 8192) {
        buff_size = 8192;
    }
    else if(free_heap > 2 * 4096) {
        buff_size = 4096;
    }
    else if(free_heap > 2 * 2048) {
        buff_size = 2048;
    }
    else if(free_heap > 2 * 1024) {
        buff_size = 1024;
    }
    else {
        buff_size = 512;
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    NYX_LOG_INFO("Free heap size: %u bytes", static_cast<uint32_t>(free_heap));
    NYX_LOG_INFO("MQTT buffer size: %u bytes", static_cast<uint32_t>(buff_size));

    /*----------------------------------------------------------------------------------------------------------------*/

    return buff_size;
}

/*--------------------------------------------------------------------------------------------------------------------*/

void internal_stack_initialize(nyx_node_t *node, uint32_t retry_ms)
{
    /*----------------------------------------------------------------------------------------------------------------*/

    auto stack = node->stack = new nyx_stack_t();

    /*----------------------------------------------------------------------------------------------------------------*/

    nyx_node = node;

    /*----------------------------------------------------------------------------------------------------------------*/

    #if defined(NYX_HAS_ETHERNET)
    if(Ethernet.linkStatus() != LinkON)
    {
        NYX_LOG_ERROR("Ethernet link down");
    }

    EthDNS.begin(Ethernet.dnsServerIP());
    #endif

    /*----------------------------------------------------------------------------------------------------------------*/

    if(node->mqtt_url != nullptr && node->mqtt_url[0] != '\0')
    {
        IPAddress ip;
        int port;

        if(_parse_host_port(node->mqtt_url, ip, port, 1883))
        {
            if(stack->mqtt_client.setBufferSize(_mqtt_buffer_size()))
            {
                NYX_LOG_INFO("MQTT ip: %d:%d:%d:%d, port: %d",
                    ip[0],
                    ip[1],
                    ip[2],
                    ip[3],
                    port
                );

                stack->mqtt_client.setCallback(
                    _mqtt_callback
                ).setServer(
                    ip, port
                );
            }
            else
            {
                NYX_LOG_ERROR("Cannot initialize MQTT client: out of memory");

                node->mqtt_url = nullptr;
            }
        }
        else
        {
            NYX_LOG_ERROR("Cannot initialize MQTT client: bad address");

            node->mqtt_url = nullptr;
        }
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    if(node->redis_url != nullptr && node->redis_url[0] != '\0')
    {
        if(_parse_host_port(node->redis_url, stack->redis_ip, stack->redis_port, 6379))
        {
            NYX_LOG_INFO("Redis ip: %d:%d:%d:%d, port: %d",
                stack->redis_ip[0],
                stack->redis_ip[1],
                stack->redis_ip[2],
                stack->redis_ip[3],
                stack->redis_port
            );
        }
        else
        {
            NYX_LOG_ERROR("Cannot initialize Redis client: bad address");

            node->redis_url = nullptr;
        }
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    if(node->mqtt_url != nullptr && node->mqtt_url[0] != '\0')
    {
        nyx_node_add_timer(node, NYX_PING_MS, _ping_timer_handler, node);
    }

    nyx_node_add_timer(node, retry_ms, _retry_timer_handler, node);

    /*----------------------------------------------------------------------------------------------------------------*/
}

/*--------------------------------------------------------------------------------------------------------------------*/

void internal_stack_finalize(__NYX_UNUSED__ nyx_node_t *node)
{
    for(;;)
    {
        delay(1);
    }
}

/*--------------------------------------------------------------------------------------------------------------------*/

struct _timer_ctx_s
{
    void (* callback)(void *arg);

    void *arg;
};

/*--------------------------------------------------------------------------------------------------------------------*/

static bool _timer_trampoline(void *ctx)
{
    auto p = static_cast<struct _timer_ctx_s *>(ctx);

    p->callback(p->arg);

    return true;
}

/*--------------------------------------------------------------------------------------------------------------------*/

void nyx_node_add_timer(nyx_node_t *node, uint32_t interval_ms, void(* callback)(void *), void *arg)
{
    if(node != nullptr && interval_ms > 0x0000U && callback != nullptr)
    {
        /*------------------------------------------------------------------------------------------------------------*/

        auto ctx = static_cast<struct _timer_ctx_s *>(nyx_memory_alloc(sizeof(struct _timer_ctx_s)));

        ctx->callback = callback;
        ctx->arg = arg;

        /*------------------------------------------------------------------------------------------------------------*/

        node->stack->timer.in(0x00000000000U, _timer_trampoline, ctx);

        node->stack->timer.every(interval_ms, _timer_trampoline, ctx);

        /*------------------------------------------------------------------------------------------------------------*/
    }
}

/*--------------------------------------------------------------------------------------------------------------------*/

void nyx_node_poll(nyx_node_t *node, uint32_t timeout_ms)
{
    if(node != nullptr)
    {
        node->stack->timer.tick();

        node->stack->mqtt_client.loop();

        delay(timeout_ms == 0 ? timeout_ms : 10);
    }
}

/*--------------------------------------------------------------------------------------------------------------------*/
#endif
/*--------------------------------------------------------------------------------------------------------------------*/
