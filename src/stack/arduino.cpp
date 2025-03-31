/* NyxNode
 * Author: Jérôme ODIER <jerome.odier@lpsc.in2p3.fr>
 * SPDX-License-Identifier: GPL-2.0-only (Mongoose backend) or GPL-3.0+
 */

/*--------------------------------------------------------------------------------------------------------------------*/
#if defined(ARDUINO)
/*--------------------------------------------------------------------------------------------------------------------*/

#include <Arduino.h>
#include <PubSubClient.h>

#if defined(HAS_WIFI)
#  if defined(ARDUINO_ARCH_ESP8266)
#    include <ESP8266WiFi.h>
#  else
#    include <WiFi.h>
#  endif
#elif defined(HAS_ETHERNET)
#  include <Dns.h>
#  include <Ethernet.h>
#else
#  error "Neither HAS_WIFI nor HAS_ETHERNET are defined!"
#endif

#include "../nyx_node_internal.h"

/*--------------------------------------------------------------------------------------------------------------------*/

#define TCP_MAX_CLIENTS 10

#define TCP_RECV_BUFF_SIZE 512U

/*--------------------------------------------------------------------------------------------------------------------*/

#ifdef HAS_WIFI
static WiFiClient tcpClient;
static WiFiServer tcpServer(0);
#endif

#ifdef HAS_ETHERNET
static EthernetClient tcpClient;
static EthernetServer tcpServer(0);
#endif

/*--------------------------------------------------------------------------------------------------------------------*/

static PubSubClient mqttClient(tcpClient);

/*--------------------------------------------------------------------------------------------------------------------*/

#if defined(HAS_ETHERNET)
static DNSClient EthDNS;
#endif

/*--------------------------------------------------------------------------------------------------------------------*/

static nyx_node_t *nyx_node;

/*--------------------------------------------------------------------------------------------------------------------*/

struct nyx_stack_s
{
    /*----------------------------------------------------------------------------------------------------------------*/

    struct TCPClient
    {
        #ifdef HAS_WIFI
        WiFiClient tcp_client;
        #endif

        #ifdef HAS_ETHERNET
        EthernetClient tcp_client;
        #endif

        __ZEROABLE__ size_t recv_capa = 0x00000;
        __ZEROABLE__ size_t recv_size = 0x00000;
        __NULLABLE__ buff_t recv_buff = nullptr;

    } clients[TCP_MAX_CLIENTS];

    /*----------------------------------------------------------------------------------------------------------------*/

    __NULLABLE__ STR_t mqtt_username = nullptr;
    __NULLABLE__ STR_t mqtt_password = nullptr;

    /*----------------------------------------------------------------------------------------------------------------*/

    unsigned long last_ping_ms;

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

        if(p != NULL)
        {
            file = p + 1;
        }
        else
        {
            p = strrchr(file, '\\');

            if(p != NULL)
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
        #if defined(ESP32) || defined(ESP8266)
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

void nyx_tcp_pub(nyx_node_t *node, nyx_str_t message)
{
    if(node->tcp_url != NULL)
    {
        auto clients = node->stack->clients;

        for(int i = 0; i < TCP_MAX_CLIENTS; i++)
        {
            auto &tcp_client = clients[i].tcp_client;

            if(tcp_client.connected())
            {
                tcp_client.write(message.buf, message.len);
            }
        }
    }
}

/*--------------------------------------------------------------------------------------------------------------------*/

void nyx_mqtt_sub(nyx_node_t *node, nyx_str_t topic)
{
    if(node->mqtt_url != NULL && mqttClient.connected())
    {
        mqttClient.subscribe(topic.buf, 1);
    }
}

/*--------------------------------------------------------------------------------------------------------------------*/

void nyx_mqtt_pub(nyx_node_t *node, nyx_str_t topic, nyx_str_t message)
{
    if(node->mqtt_url != NULL && mqttClient.connected())
    {
        mqttClient.publish(topic.buf, reinterpret_cast<uint8_t *>(message.buf), reinterpret_cast<unsigned int>(message.len));
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

    size_t proto_sep = url.indexOf("://");

    if(proto_sep >= 0)
    {
        url = url.substring(proto_sep + 3);
    }

    /*----------------------------------------------------------------------------------------------------------------*/
    /* GET HOST AND PORT                                                                                              */
    /*----------------------------------------------------------------------------------------------------------------*/

    String host;

    size_t colon = url.indexOf(':');

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

    #ifdef HAS_WIFI
    return ip.fromString(host.c_str()) || WiFi.hostByName(host.c_str(), ip) == 1;
    #endif

    #ifdef HAS_ETHERNET
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

    nyx_node->mqtt_handler(nyx_node, NYX_EVENT_MSG, nyx_str_s(topic), message);
}

/*--------------------------------------------------------------------------------------------------------------------*/

void nyx_node_stack_initialize(
    nyx_node_t *node,
    __NULLABLE__ STR_t mqtt_username,
    __NULLABLE__ STR_t mqtt_password,
    int retry_ms
) {
    /*----------------------------------------------------------------------------------------------------------------*/

    nyx_stack_t *stack = node->stack = static_cast<nyx_stack_t *>(nyx_memory_alloc(sizeof(nyx_stack_t)));

    memset(stack, 0x00, sizeof(struct nyx_stack_s));

    /*----------------------------------------------------------------------------------------------------------------*/

    stack->mqtt_username = mqtt_username;
    stack->mqtt_password = mqtt_password;

    /*----------------------------------------------------------------------------------------------------------------*/

    nyx_node = node;

    /*----------------------------------------------------------------------------------------------------------------*/

    if(node->tcp_url != NULL)
    {
        IPAddress ip;
        int port;

        if(parse_host_port(node->tcp_url, ip, port, 7624))
        {
            NYX_LOG_INFO("TCP ip: %d:%d:%d:%d, port: %d", ip[0], ip[1], ip[2], ip[3], port);

            #ifdef HAS_WIFI
            tcpServer = WiFiServer(ip, port);
            #endif

            #ifdef HAS_ETHERNET
            tcpServer = EthernetServer(/**/port/**/);
            #endif

            tcpServer.begin();
        }
        else
        {
            node->tcp_url = nullptr;
        }
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    if(node->mqtt_url != NULL)
    {
        IPAddress ip;
        int port;

        if(parse_host_port(node->mqtt_url, ip, port, 1883))
        {
            NYX_LOG_INFO("MQTT ip: %d:%d:%d:%d, port: %d", ip[0], ip[1], ip[2], ip[3], port);

            mqttClient.setCallback(
                mqtt_callback
            );

            mqttClient.setServer(
                ip, port
            );
        }
        else
        {
            node->mqtt_url = nullptr;
        }
    }

    /*----------------------------------------------------------------------------------------------------------------*/
}

/*--------------------------------------------------------------------------------------------------------------------*/

void nyx_node_stack_finalize(nyx_node_t *node)
{
    for(;;)
    {
        delay(1);
    }
}

/*--------------------------------------------------------------------------------------------------------------------*/

static void read_data(nyx_stack_s::TCPClient &client)
{
    /*----------------------------------------------------------------------------------------------------------------*/

    size_t available = client.tcp_client.available();

    if(available == 0)
    {
        return;
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    size_t required = client.recv_size + available;

    if(required > client.recv_capa)
    {
        size_t new_capa = max(required, client.recv_capa + TCP_RECV_BUFF_SIZE);

        buff_t new_buff = nyx_memory_realloc(client.recv_buff, new_capa);

        client.recv_capa = new_capa;
        client.recv_buff = new_buff;
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    client.recv_size += static_cast<size_t>(client.tcp_client.read(static_cast<uint8_t *>(client.recv_buff) + client.recv_size, available));

    /*----------------------------------------------------------------------------------------------------------------*/
}

/*--------------------------------------------------------------------------------------------------------------------*/

static void consume_data(nyx_node_t *node, nyx_stack_s::TCPClient &client)
{
    if(client.recv_size > 0)
    {
        /*------------------------------------------------------------------------------------------------------------*/
        /* COMSUME DATA                                                                                               */
        /*------------------------------------------------------------------------------------------------------------*/

        size_t consumed = node->tcp_handler(node, NYX_EVENT_MSG, client.recv_size, client.recv_buff);

        if(consumed > client.recv_size)
        {
            NYX_LOG_ERROR("Internal error");

            return;
        }

        /*------------------------------------------------------------------------------------------------------------*/

        if(consumed > 0)
        {
            /*--------------------------------------------------------------------------------------------------------*/
            /* MOVE DATA                                                                                              */
            /*--------------------------------------------------------------------------------------------------------*/

            memmove(client.recv_buff, static_cast<uint8_t *>(client.recv_buff) + consumed, client.recv_size = client.recv_size - consumed);

            /*--------------------------------------------------------------------------------------------------------*/
            /* SHRINK BUFFER                                                                                          */
            /*--------------------------------------------------------------------------------------------------------*/

            const size_t shrink_threshold = client.recv_capa / 2;

            if(client.recv_capa > TCP_RECV_BUFF_SIZE && client.recv_size < shrink_threshold)
            {
                size_t new_capa = max(TCP_RECV_BUFF_SIZE, (client.recv_size * 5) / 4);

                buff_t new_buff = nyx_memory_realloc(client.recv_buff, new_capa);

                client.recv_capa = new_capa;
                client.recv_buff = new_buff;
            }

            /*--------------------------------------------------------------------------------------------------------*/
        }

        /*------------------------------------------------------------------------------------------------------------*/
    }
}

/*--------------------------------------------------------------------------------------------------------------------*/

void nyx_stack_poll(nyx_node_t *node, int timeout_ms)
{
    auto stack = node->stack;

    auto clients = stack->clients;

    /*----------------------------------------------------------------------------------------------------------------*/
    /* TCP                                                                                                            */
    /*----------------------------------------------------------------------------------------------------------------*/

    if(node->tcp_url != NULL)
    {
        /*------------------------------------------------------------------------------------------------------------*/
        /* CLEANUP OLD CLIENTS & REGISTER NEW CLIENTS                                                                 */
        /*------------------------------------------------------------------------------------------------------------*/

        #ifdef HAS_WIFI
        WiFiClient new_client = tcpServer.accept();
        #endif

        #ifdef HAS_ETHERNET
        EthernetClient new_client = tcpServer.accept();
        #endif

        /*------------------------------------------------------------------------------------------------------------*/

        bool accepted = false;

        for(int i = 0; i < TCP_MAX_CLIENTS; ++i)
        {
            auto &client = clients[i];

            if(!client.tcp_client.connected())
            {
                /*----------------------------------------------------------------------------------------------------*/

                client.tcp_client.stop();

                nyx_memory_free(client.recv_buff);

                client.recv_capa = 0x00000;
                client.recv_size = 0x00000;
                client.recv_buff = nullptr;

                /*----------------------------------------------------------------------------------------------------*/

                if(!accepted && new_client.connected())
                {
                    client.tcp_client = new_client;

                    accepted = true;
                }

                /*----------------------------------------------------------------------------------------------------*/
            }
        }

        /*------------------------------------------------------------------------------------------------------------*/

        if(!accepted && new_client.connected())
        {
            new_client.stop();
        }

        /*------------------------------------------------------------------------------------------------------------*/
        /* RECIEVE DATA                                                                                               */
        /*------------------------------------------------------------------------------------------------------------*/

        for(int i = 0; i < TCP_MAX_CLIENTS; i++)
        {
            auto &client = clients[i];

            if(client.tcp_client.connected())
            {
                read_data(client);

                consume_data(node, client);
            }
        }

        /*------------------------------------------------------------------------------------------------------------*/
    }

    /*----------------------------------------------------------------------------------------------------------------*/
    /* MQTT                                                                                                           */
    /*----------------------------------------------------------------------------------------------------------------*/

    if(node->mqtt_url != NULL)
    {
        /*------------------------------------------------------------------------------------------------------------*/
        /* RECONNECT CLIENT                                                                                           */
        /*------------------------------------------------------------------------------------------------------------*/

        if(!mqttClient.connected())
        {
            if(mqttClient.connect(node->node_id.buf, stack->mqtt_username, stack->mqtt_password))
            {
                node->mqtt_handler(node, NYX_EVENT_OPEN, node->node_id, node->node_id);
            }
            else
            {
                delay(timeout_ms);

                return;
            }
        }

        /*------------------------------------------------------------------------------------------------------------*/
        /* LOOP                                                                                                       */
        /*------------------------------------------------------------------------------------------------------------*/

        mqttClient.loop();

        /*------------------------------------------------------------------------------------------------------------*/

        unsigned long curr_ping_ms = millis();

        if(stack->last_ping_ms <= curr_ping_ms - NYX_PING_MS)
        {
            stack->last_ping_ms = curr_ping_ms - 0x0000000UL;

            nyx_mqtt_pub(node, nyx_str_s("nyx/ping/node"), node->node_id);

            nyx_mqtt_pub(node, node->master_client_topic, node->master_client_message);
        }

        /*------------------------------------------------------------------------------------------------------------*/
    }

    /*----------------------------------------------------------------------------------------------------------------*/
}

/*--------------------------------------------------------------------------------------------------------------------*/
#endif
/*--------------------------------------------------------------------------------------------------------------------*/
