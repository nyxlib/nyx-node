/*--------------------------------------------------------------------------------------------------------------------*/
#if defined(ARDUINO)
/*--------------------------------------------------------------------------------------------------------------------*/

#include <Arduino.h>
#include <PubSubClient.h>

#if defined(ARDUINO_ARCH_ESP32)
#  include <WiFi.h>
#elif defined(ARDUINO_ARCH_ESP8266)
#  include <ESP8266WiFi.h>
#else
#  define ETHERNET

#  include <Dns.h>
#  include <Ethernet.h>

static DNSClient ethDNS;
#endif

#include "../nyx_node_internal.h"

/*--------------------------------------------------------------------------------------------------------------------*/

#define TCP_MAX_CLIENTS 10

#define TCP_RECV_BUFF_SIZE 512U

/*--------------------------------------------------------------------------------------------------------------------*/

#ifndef ETHERNET
static WiFiClient tcpClient;

static WiFiServer tcpServer(0);
#else
static EthernetClient tcpClient;

static EthernetServer tcpServer(0);
#endif

static PubSubClient mqttClient(tcpClient);

/*--------------------------------------------------------------------------------------------------------------------*/

struct nyx_stack_s
{
    /*----------------------------------------------------------------------------------------------------------------*/

    struct TCPClient
    {
        #ifndef ETHERNET
        WiFiClient tcp_client;
        #else
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
};

/*--------------------------------------------------------------------------------------------------------------------*/
/* LOGGER                                                                                                             */
/*--------------------------------------------------------------------------------------------------------------------*/

void nyx_log_prefix(nyx_log_level_t level, STR_t file, STR_t func, int line)
{
    /*----------------------------------------------------------------------------------------------------------------*/

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

    /*----------------------------------------------------------------------------------------------------------------*/

    printf("%s - %s: %d %s() - ", nyx_log_level_to_str(level), file, line, func);

    /*----------------------------------------------------------------------------------------------------------------*/
}

/*--------------------------------------------------------------------------------------------------------------------*/

void nyx_log(const char *fmt, ...)
{
    /*----------------------------------------------------------------------------------------------------------------*/

    va_list ap;
    va_start(ap, fmt);
    vprintf(fmt, ap);
    va_end(ap);

    /*----------------------------------------------------------------------------------------------------------------*/

    putchar('\r');
    putchar('\n');

    /*----------------------------------------------------------------------------------------------------------------*/

    fflush(stdout);

    /*----------------------------------------------------------------------------------------------------------------*/
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

void nyx_mqtt_sub(nyx_node_t *node, nyx_str_t topic, int qos)
{
    if(node->mqtt_url != NULL && mqttClient.connected())
    {
        mqttClient.subscribe(topic.buf, qos);
    }
}

/*--------------------------------------------------------------------------------------------------------------------*/

void nyx_mqtt_pub(nyx_node_t *node, nyx_str_t topic, nyx_str_t message, int qos, bool retain)
{
    if(node->mqtt_url != NULL && mqttClient.connected())
    {
        mqttClient.publish(topic.buf, reinterpret_cast<uint8_t *>(message.buf), reinterpret_cast<unsigned int>(message.len), retain);
    }
}

/*--------------------------------------------------------------------------------------------------------------------*/
/* STACK                                                                                                              */
/*--------------------------------------------------------------------------------------------------------------------*/

static bool parse_host_port(const String &_url, IPAddress &ip, int &port, int default_port)
{
    String url = _url;

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

    #ifndef ETHERNET
    return ip.fromString(host.c_str()) || WiFi.hostByName(host.c_str(), ip) == 1;
    #else
    return ip.fromString(host.c_str()) || ethDNS.getHostByName(host.c_str(), ip) == 1;
    #endif

    /*----------------------------------------------------------------------------------------------------------------*/
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

    /*----------------------------------------------------------------------------------------------------------------*/

    stack->mqtt_username = mqtt_username;
    stack->mqtt_password = mqtt_password;

    /*----------------------------------------------------------------------------------------------------------------*/

    auto mqtt_callback = [node](char *topic, uint8_t *buff, unsigned int size)
    {
        nyx_str_t message = {reinterpret_cast<str_t>(buff), reinterpret_cast<size_t>(size)};

        node->mqtt_handler(node, NYX_EVENT_MSG, nyx_str_s(topic), message);
    };

    /*----------------------------------------------------------------------------------------------------------------*/

    if(node->tcp_url != NULL)
    {
        IPAddress ip;
        int port;

        if(parse_host_port(node->tcp_url, ip, port, 7624))
        {
            NYX_INFO(("TCP ip: %s, port: %d:%d:%d:%d", ip[0], ip[1], ip[2], ip[3], port));

            #ifndef ETHERNET
            tcpServer = WiFiServer(ip, port);
            #else
            tcpServer = EthernetServer(/**/ port);
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
            NYX_INFO(("MQTT ip: %s, port: %d:%d:%d:%d", ip[0], ip[1], ip[2], ip[3], port));

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
        exit(0);
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
            NYX_ERROR(("Internal error"));

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

        #ifndef ETHERNET
        WiFiClient new_client = tcpServer.accept();
        #else
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

        mqttClient.loop();

        /*------------------------------------------------------------------------------------------------------------*/
    }

    /*----------------------------------------------------------------------------------------------------------------*/
}

/*--------------------------------------------------------------------------------------------------------------------*/
#endif
/*--------------------------------------------------------------------------------------------------------------------*/
