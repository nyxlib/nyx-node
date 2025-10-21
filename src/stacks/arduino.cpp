/* NyxNode
 * Author: Jérôme ODIER <jerome.odier@lpsc.in2p3.fr>
 * SPDX-License-Identifier: GPL-2.0-only (Mongoose backend) or GPL-3.0+
 */

/*--------------------------------------------------------------------------------------------------------------------*/
#if defined(ARDUINO)
/*--------------------------------------------------------------------------------------------------------------------*/

#include <Arduino.h>
#include <PubSubClient.h>
#include <vector>
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

#define TCP_RECV_BUFF_GROW_STEP         512U
#define TCP_RECV_BUFF_SHRINK_FACTOR     1.25f

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

static bool __nyx_timer_trampoline(void *ctx)
{
    nyx_timer_ctx_t *p = (nyx_timer_ctx_t *) ctx;

    p->cb(p->arg);

    return true;
}

/*--------------------------------------------------------------------------------------------------------------------*/
static void ping_timer_handler(void *arg)
{
    nyx_node_ping((nyx_node_t *) arg);
}

/*--------------------------------------------------------------------------------------------------------------------*/

struct nyx_stack_s
{
    /*----------------------------------------------------------------------------------------------------------------*/

    #ifdef NYX_HAS_WIFI
    WiFiClient tcp_client;
    WiFiServer tcp_server;

    WiFiClient indi_client;
    WiFiClient redis_client;
    #endif

    #ifdef NYX_HAS_ETHERNET
    EthernetClient tcp_client;
    EthernetServer tcp_server;

    EthernetClient indi_client;
    EthernetClient redis_client;
    #endif

    /*----------------------------------------------------------------------------------------------------------------*/

    PubSubClient mqtt_client;

    /*----------------------------------------------------------------------------------------------------------------*/

    IPAddress indi_ip;
    int indi_port = 7624;

    IPAddress redis_ip;
    int redis_port = 6379;

    /*----------------------------------------------------------------------------------------------------------------*/

    __NULLABLE__ STR_t mqtt_username = nullptr;
    __NULLABLE__ STR_t mqtt_password = nullptr;

    __NULLABLE__ STR_t redis_username = nullptr;
    __NULLABLE__ STR_t redis_password = nullptr;

    /*----------------------------------------------------------------------------------------------------------------*/

    std::vector<uint8_t> recv_buff;

    /*----------------------------------------------------------------------------------------------------------------*/

    bool indi_server_started = false;

    /*----------------------------------------------------------------------------------------------------------------*/

    uint16_t mqtt_buf_estimate = 0;

    /*----------------------------------------------------------------------------------------------------------------*/

    nyx_stack_s():
        tcp_client(),
        tcp_server(),
        mqtt_client(tcp_client),
        indi_client(),
        redis_client(),
        recv_buff(),
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

void internal_indi_pub(nyx_node_t *node, const nyx_str_t message)
{
    auto stack = node->stack;

    if(stack->indi_client.connected())
    {
        stack->indi_client.write(message.buf, message.len);
    }
}

/*--------------------------------------------------------------------------------------------------------------------*/

void internal_mqtt_sub(nyx_node_t *node, const nyx_str_t topic)
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

void internal_mqtt_pub(nyx_node_t *node, const nyx_str_t topic, const nyx_str_t message)
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

static bool parse_host_port(String url, IPAddress &ip, int &port, int default_port)
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

static void mqtt_callback(char *topic, uint8_t *buff, unsigned int size)
{
    nyx_str_t message = {
        reinterpret_cast<str_t>(buff),
        reinterpret_cast<size_t>(size),
    };

    nyx_node->mqtt_handler(nyx_node, NYX_NODE_EVENT_MSG, nyx_str_s(topic), message);
}

/*--------------------------------------------------------------------------------------------------------------------*/

static uint16_t mqtt_estimate_buffer_size()
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

    NYX_LOG_INFO("Free heap size: %u bytes", (uint32_t) free_heap);
    NYX_LOG_INFO("MQTT buffer size: %u bytes", (uint32_t) buff_size);

    /*----------------------------------------------------------------------------------------------------------------*/

    return buff_size;
}

/*--------------------------------------------------------------------------------------------------------------------*/

void nyx_node_stack_initialize(
    nyx_node_t *node,
    __NULLABLE__ STR_t mqtt_username,
    __NULLABLE__ STR_t mqtt_password,
    __NULLABLE__ STR_t redis_username,
    __NULLABLE__ STR_t redis_password,
    __UNUSED__ int retry_ms
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

    stack->mqtt_buf_estimate = mqtt_estimate_buffer_size();

    /*----------------------------------------------------------------------------------------------------------------*/

    if(node->indi_url != nullptr && node->indi_url[0] != '\0')
    {
        if(parse_host_port(node->indi_url, stack->indi_ip, stack->indi_port, 7624))
        {
            NYX_LOG_INFO("INDI ip: %d:%d:%d:%d, port: %d",
                stack->indi_ip[0],
                stack->indi_ip[1],
                stack->indi_ip[2],
                stack->indi_ip[3],
                stack->indi_port
            );

            stack->recv_buff.clear();
            stack->recv_buff.reserve(stack->mqtt_buf_estimate);
        }
        else
        {
            NYX_LOG_ERROR("Cannot initialize TCP server: bad address");

            node->indi_url = nullptr;
        }
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    if(node->mqtt_url != nullptr && node->mqtt_url[0] != '\0')
    {
        IPAddress ip;
        int port;

        if(parse_host_port(node->mqtt_url, ip, port, 1883))
        {
            if(stack->mqtt_client.setBufferSize(stack->mqtt_buf_estimate))
            {
                NYX_LOG_INFO("MQTT ip: %d:%d:%d:%d, port: %d", ip[0], ip[1], ip[2], ip[3], port);

                stack->mqtt_client.setCallback(
                    mqtt_callback
                ).setServer(
                    ip, port
                );

                nyx_node_add_timer(node, NYX_PING_MS, ping_timer_handler, node);
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
        if(parse_host_port(node->redis_url, stack->redis_ip, stack->redis_port, 6379))
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

void nyx_node_stack_finalize(__UNUSED__ nyx_node_t *node)
{
    for(;;)
    {
        delay(1);
    }
}

/*--------------------------------------------------------------------------------------------------------------------*/

static void read_data(nyx_node_t *node, size_t grow_step, float shrink_factor)
{
    auto stack = node->stack;

    /*----------------------------------------------------------------------------------------------------------------*/
    /* READ AVAILABLE DATA                                                                                            */
    /*----------------------------------------------------------------------------------------------------------------*/

    size_t available = stack->indi_client.available();

    if(available == 0)
    {
        return;
    }

    /*----------------------------------------------------------------------------------------------------------------*/
    /* EXPAND BUFFER IF NEEDED (STEP)                                                                                 */
    /*----------------------------------------------------------------------------------------------------------------*/

    const size_t required = stack->recv_buff.size() + available;

    if(required > stack->recv_buff.capacity())
    {
        size_t new_capa = stack->recv_buff.capacity() + grow_step;

        if(new_capa < required)
        {
            new_capa = required;
        }

        std::vector<uint8_t> tmp;
        tmp.reserve(new_capa);
        tmp.insert(tmp.end(), stack->recv_buff.begin(), stack->recv_buff.end());
        stack->recv_buff.swap(tmp);
    }

    /*----------------------------------------------------------------------------------------------------------------*/
    /* CONSUME DATA                                                                                                   */
    /*----------------------------------------------------------------------------------------------------------------*/

    size_t old_size = stack->recv_buff.size();

    stack->recv_buff.resize(old_size + available);

    size_t got = stack->indi_client.read(
        static_cast<uint8_t *>(stack->recv_buff.data()) + old_size,
        available
    );

    if(got < available)
    {
        stack->recv_buff.resize(old_size + got);
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    if(!stack->recv_buff.empty())
    {
        size_t consumed = node->tcp_handler(
            node,
            NYX_NODE_EVENT_MSG,
            NYX_STR_S(reinterpret_cast<str_t>(stack->recv_buff.data()), stack->recv_buff.size())
        );

        if(consumed > stack->recv_buff.size())
        {
            NYX_LOG_ERROR("Internal error: consumed > size");

            return;
        }

        if(consumed > 0)
        {
            stack->recv_buff.erase(
                stack->recv_buff.begin(),
                stack->recv_buff.begin() + static_cast<long>(consumed)
            );

            const size_t capa = stack->recv_buff.capacity();
            const size_t size = stack->recv_buff.size();

            if(capa > grow_step && size < (capa / 2))
            {
                size_t new_capa = max(grow_step, static_cast<size_t>(static_cast<float>(size) * shrink_factor));

                std::vector<uint8_t> tmp;
                tmp.reserve(new_capa);
                tmp.insert(tmp.end(), stack->recv_buff.begin(), stack->recv_buff.end());
                stack->recv_buff.swap(tmp);
            }
        }
    }

    /*----------------------------------------------------------------------------------------------------------------*/
}

/*--------------------------------------------------------------------------------------------------------------------*/

void nyx_node_add_timer(nyx_node_t *node, uint64_t interval_ms, void(* callback)(void *), void *arg)
{
    if(node != NULL && callback != NULL)
    {
        nyx_timer_ctx_t *ctx = (nyx_timer_ctx_t *) nyx_memory_alloc(sizeof(nyx_timer_ctx_t));

        if(ctx == NULL)
        {
            NYX_LOG_ERROR("Cannot create timer: out of memory");

            return;
        }

        ctx->cb  = callback;
        ctx->arg = arg;

        __nyx_timer.in(
            0,
            __nyx_timer_trampoline,
            (void *) ctx
        );

        uint32_t ival = (uint32_t) (interval_ms > 0xFFFFFFFFULL ? 0xFFFFFFFFUL : interval_ms);

        __nyx_timer.every(
            ival,
            __nyx_timer_trampoline,
            (void *) ctx
        );
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
    /* TCP                                                                                                            */
    /*----------------------------------------------------------------------------------------------------------------*/

    if(node->indi_url != nullptr && node->indi_url[0] != '\0')
    {
        /*------------------------------------------------------------------------------------------------------------*/
        /* START SERVER ONCE                                                                                          */
        /*------------------------------------------------------------------------------------------------------------*/

        if(!stack->indi_server_started)
        {
            stack->tcp_server.begin(stack->indi_port);

            stack->indi_server_started = true;

            NYX_LOG_INFO("INDI support is enabled");
        }

        /*------------------------------------------------------------------------------------------------------------*/
        /* ACCEPT CLIENT (PORTABLE)                                                                                   */
        /*------------------------------------------------------------------------------------------------------------*/

        if(!stack->indi_client.connected())
        {
            #ifdef NYX_HAS_WIFI
            WiFiClient new_client = stack->tcp_server.available();
            #endif

            #ifdef NYX_HAS_ETHERNET
            EthernetClient new_client = stack->tcp_server.available();
            #endif

            if(new_client.connected())
            {
                stack->indi_client = new_client;

                stack->recv_buff.clear();
                stack->recv_buff.reserve(stack->mqtt_buf_estimate);
            }
            else
            {
                goto __mqtt;
            }
        }

        /*------------------------------------------------------------------------------------------------------------*/
        /* TREATMENT                                                                                                  */
        /*------------------------------------------------------------------------------------------------------------*/

        read_data(node, TCP_RECV_BUFF_GROW_STEP, TCP_RECV_BUFF_SHRINK_FACTOR);
    }

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
                node->mqtt_handler(node, NYX_EVENT_OPEN, node->node_id, node->node_id);

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
