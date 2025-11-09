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
/* TIMERS (LITE: ARDUINO-TIMER + TRAMPOLINE)                                                                          */
/*--------------------------------------------------------------------------------------------------------------------*/

static auto __nyx_timer = timer_create_default();

/*--------------------------------------------------------------------------------------------------------------------*/

typedef struct nyx_timer_ctx_s
{
    void (* cb)(void *);
    void *arg;
}
nyx_timer_ctx_t;

/*--------------------------------------------------------------------------------------------------------------------*/

static bool _timer_trampoline(void *ctx)
{
    auto *p = static_cast<nyx_timer_ctx_t *>(ctx);

    p->cb(p->arg);

    return true;
}

/*--------------------------------------------------------------------------------------------------------------------*/

static void _ping_timer_handler(void *arg)
{
    nyx_node_ping(static_cast<nyx_node_t *>(arg));
}

/*--------------------------------------------------------------------------------------------------------------------*/

struct nyx_stack_s
{
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

    __NYX_NULLABLE__ STR_t mqtt_username = nullptr;
    __NYX_NULLABLE__ STR_t mqtt_password = nullptr;

    __NYX_NULLABLE__ STR_t redis_username = nullptr;
    __NYX_NULLABLE__ STR_t redis_password = nullptr;

    /*----------------------------------------------------------------------------------------------------------------*/

    bool indi_server_started = false;

    /*----------------------------------------------------------------------------------------------------------------*/

    uint16_t mqtt_buf_estimate = 0;

    /*----------------------------------------------------------------------------------------------------------------*/

    nyx_stack_s():
        tcp_client(),
        mqtt_client(tcp_client),
        redis_client(),
        indi_server_started(false),
        mqtt_buf_estimate(0)
    {}

    /*----------------------------------------------------------------------------------------------------------------*/
};

/*--------------------------------------------------------------------------------------------------------------------*/
/* LOGGER                                                                                                             */
/*--------------------------------------------------------------------------------------------------------------------*/

void nyx_log(nyx_log_level_t level, STR_t file, STR_t func, int line, const char *fmt, ...)
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

void internal_mqtt_sub(nyx_node_t *node, const nyx_str_t topic, int qos)
{
    auto stack = node->stack;

    if(stack->mqtt_client.connected())
    {
        if(!stack->mqtt_client.subscribe(topic.buf, 1))
        {
            NYX_LOG_ERROR("Cannot subscribe to %s", topic.buf);
        }
    }
}

/*--------------------------------------------------------------------------------------------------------------------*/

void internal_mqtt_pub(nyx_node_t *node, const nyx_str_t topic, const nyx_str_t message, int qos)
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

void internal_redis_pub(nyx_node_t *node, const nyx_str_t message)
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
        port = atoi(url.substring(colon + 1).c_str());
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

static uint16_t _mqtt_estimate_buffer_size()
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

void internal_stack_initialize(
    nyx_node_t *node,
    __NYX_NULLABLE__ STR_t mqtt_username,
    __NYX_NULLABLE__ STR_t mqtt_password,
    __NYX_NULLABLE__ STR_t redis_username,
    __NYX_NULLABLE__ STR_t redis_password,
    uint64_t retry_ms
) {
    /*----------------------------------------------------------------------------------------------------------------*/

    nyx_stack_t *stack = node->stack = new nyx_stack_t();

    /*----------------------------------------------------------------------------------------------------------------*/

    stack->mqtt_username = mqtt_username;
    stack->mqtt_password = mqtt_password;
    stack->redis_username = redis_username;
    stack->redis_password = redis_password;

    /*----------------------------------------------------------------------------------------------------------------*/

    nyx_node = node;

    /*----------------------------------------------------------------------------------------------------------------*/

    stack->mqtt_buf_estimate = _mqtt_estimate_buffer_size();

    /*----------------------------------------------------------------------------------------------------------------*/

    if(node->mqtt_url != nullptr && node->mqtt_url[0] != '\0')
    {
        IPAddress ip;
        int port;

        if(_parse_host_port(node->mqtt_url, ip, port, 1883))
        {
            if(stack->mqtt_client.setBufferSize(stack->mqtt_buf_estimate))
            {
                NYX_LOG_INFO("MQTT ip: %d:%d:%d:%d, port: %d", ip[0], ip[1], ip[2], ip[3], port);

                stack->mqtt_client.setCallback(
                    _mqtt_callback
                ).setServer(
                    ip, port
                );

                nyx_node_add_timer(node, NYX_PING_MS, _ping_timer_handler, node);
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

void nyx_node_add_timer(nyx_node_t *node, uint64_t interval_ms, void(* callback)(void *), void *arg)
{
    if(node != nullptr && callback != nullptr)
    {
        /*------------------------------------------------------------------------------------------------------------*/

        auto *ctx = static_cast<nyx_timer_ctx_t *>(nyx_memory_alloc(sizeof(nyx_timer_ctx_t)));

        if(ctx == nullptr)
        {
            NYX_LOG_ERROR("Cannot create timer: out of memory");

            return;
        }

        ctx->cb = callback;
        ctx->arg = arg;

        /*------------------------------------------------------------------------------------------------------------*/

        __nyx_timer.in(0, _timer_trampoline, (void *) ctx);

        /*------------------------------------------------------------------------------------------------------------*/

        auto ival = static_cast<uint32_t>(interval_ms > 0xFFFFFFFFULL ? 0xFFFFFFFFUL : interval_ms);

        __nyx_timer.every(
            ival,
            _timer_trampoline,
            (void *) ctx
        );

        /*------------------------------------------------------------------------------------------------------------*/
    }
}

/*--------------------------------------------------------------------------------------------------------------------*/

void nyx_node_poll(nyx_node_t *node, int timeout_ms)
{
    auto stack = node->stack;

    /*----------------------------------------------------------------------------------------------------------------*/
    /* TIMERS                                                                                                         */
    /*----------------------------------------------------------------------------------------------------------------*/

    __nyx_timer.tick();

    /*----------------------------------------------------------------------------------------------------------------*/
    /* MQTT                                                                                                           */
    /*----------------------------------------------------------------------------------------------------------------*/

__mqtt:
    if(node->mqtt_url != nullptr && node->mqtt_url[0] != '\0')
    {
        if(!stack->mqtt_client.connected())
        {
            if(stack->mqtt_client.connect(node->node_id.buf, stack->mqtt_username, stack->mqtt_password))
            {
                node->mqtt_handler(node, NYX_NODE_EVENT_OPEN, node->node_id, node->node_id);

                NYX_LOG_INFO("MQTT support is enabled");
            }
            else
            {
                goto __redis;
            }
        }

        stack->mqtt_client.loop();
    }

    /*----------------------------------------------------------------------------------------------------------------*/
    /* REDIS                                                                                                          */
    /*----------------------------------------------------------------------------------------------------------------*/

__redis:
    if(node->redis_url != nullptr && node->redis_url[0] != '\0')
    {
        if(!stack->redis_client.connected())
        {
            if(stack->redis_client.connect(stack->redis_ip, stack->redis_port))
            {
                nyx_redis_auth(node, stack->redis_username, stack->redis_password);

                NYX_LOG_INFO("Redis support is enabled");
            }
            else
            {
                goto __delay;
            }
        }

        /* NOTHING TO DO */
    }

    /*----------------------------------------------------------------------------------------------------------------*/
    /* DELAY                                                                                                          */
    /*----------------------------------------------------------------------------------------------------------------*/

__delay:
    delay(timeout_ms);
}

/*--------------------------------------------------------------------------------------------------------------------*/
#endif
/*--------------------------------------------------------------------------------------------------------------------*/
