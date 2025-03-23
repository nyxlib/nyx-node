/*--------------------------------------------------------------------------------------------------------------------*/
#if defined(ARDUINO)
/*--------------------------------------------------------------------------------------------------------------------*/

#include <string>

#ifndef ESP8266
#  include <WiFi.h>
#else
#  include <ESP8266WiFi.h>
#endif

#include <Arduino.h>
#include <PubSubClient.h>

#include "../nyx_node_internal.h"

/*--------------------------------------------------------------------------------------------------------------------*/

#define MAX_TCP_CLIENTS 10

#define RECV_BUFF_SIZE 512

/*--------------------------------------------------------------------------------------------------------------------*/

struct nyx_stack_s
{
    /*----------------------------------------------------------------------------------------------------------------*/

    WiFiServer tcp_server;

    /*----------------------------------------------------------------------------------------------------------------*/

    struct tcp_client_s
    {
        WiFiClient tcp_client;

        size_t recv_size = 0x00000;
        buff_t recv_buff = nullptr;
        size_t recv_capa = 0x00000;

    } clients[MAX_TCP_CLIENTS];

    /*----------------------------------------------------------------------------------------------------------------*/

    PubSubClient mqtt_client;

    __NULLABLE__ STR_t mqtt_username = nullptr;
    __NULLABLE__ STR_t mqtt_password = nullptr;

    /*----------------------------------------------------------------------------------------------------------------*/
};

/*--------------------------------------------------------------------------------------------------------------------*/
/* LOGGER                                                                                                             */
/*--------------------------------------------------------------------------------------------------------------------*/
/*
extern "C" int putchar(int c)
{
    Serial.write((char) c);

    return c;
}
*/
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
}

/*--------------------------------------------------------------------------------------------------------------------*/
/* TCP & MQTT                                                                                                         */
/*--------------------------------------------------------------------------------------------------------------------*/

void nyx_tcp_pub(nyx_node_t *node, STR_t message)
{
    auto clients = node->stack->clients;

    for(int i = 0; i < MAX_TCP_CLIENTS; i++)
    {
        WiFiClient tcp_client = clients[i].tcp_client;

        if(tcp_client && tcp_client.connected())
        {
            tcp_client.write(message, strlen(message));
        }
    }
}

/*--------------------------------------------------------------------------------------------------------------------*/

void nyx_mqtt_sub(nyx_node_t *node, nyx_str_t topic, int qos)
{
    if(node->mqtt_url != NULL && node->stack->mqtt_client.connected())
    {
        node->stack->mqtt_client.subscribe(topic.buf, qos);
    }
}

/*--------------------------------------------------------------------------------------------------------------------*/

void nyx_mqtt_pub(nyx_node_t *node, nyx_str_t topic, nyx_str_t message, int qos, bool retain)
{
    if(node->mqtt_url != NULL && node->stack->mqtt_client.connected())
    {
        node->stack->mqtt_client.publish(topic.buf, reinterpret_cast<uint8_t *>(message.buf), reinterpret_cast<unsigned int>(message.len), retain);
    }
}

/*--------------------------------------------------------------------------------------------------------------------*/
/* STACK                                                                                                              */
/*--------------------------------------------------------------------------------------------------------------------*/

static bool parse_host_port(const std::string &url, IPAddress &ip, int &port, int default_port)
{
    /*----------------------------------------------------------------------------------------------------------------*/

    std::string host;

    size_t colon = url.find(':');

    if(colon == std::string::npos)
    {
        host = url;
        port = default_port;
    }
    else
    {
        host = url.substr(0, colon);
        port = atoi(url.substr(colon + 1).c_str());
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    return ip.fromString(host.c_str()) || WiFi.hostByName(host.c_str(), ip) == 1;

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

    auto mqtt_callback = [node](STR_t topic, byte *buff, unsigned int size)
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
            stack->tcp_server = WiFiServer(ip, port);

            stack->tcp_server.begin();
        }
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    if(node->mqtt_url != NULL)
    {
        IPAddress ip;
        int port;

        if(parse_host_port(node->mqtt_url, ip, port, 1883))
        {
            stack->mqtt_client.setCallback(
                mqtt_callback
            );

            stack->mqtt_client.setServer(
                ip, port
            );
        }
    }

    /*----------------------------------------------------------------------------------------------------------------*/
}

/*--------------------------------------------------------------------------------------------------------------------*/

void nyx_node_stack_finalize(nyx_node_t *node)
{
    /*----------------------------------------------------------------------------------------------------------------*/
    /* FINALIZE TCP                                                                                                   */
    /*----------------------------------------------------------------------------------------------------------------*/

    auto clients = node->stack->clients;

    for(int i = 0; i < MAX_TCP_CLIENTS; i++)
    {
        clients[i].tcp_client.stop();

        nyx_memory_free(
            clients[i].recv_buff
        );
    }

    node->stack->tcp_server.stop();

    /*----------------------------------------------------------------------------------------------------------------*/
    /* FINALIZE MQTT                                                                                                  */
    /*----------------------------------------------------------------------------------------------------------------*/

    node->stack->mqtt_client.disconnect();

    /*----------------------------------------------------------------------------------------------------------------*/
    /* FINALIZE STACK                                                                                                 */
    /*----------------------------------------------------------------------------------------------------------------*/

    nyx_memory_free(node->stack);

    /*----------------------------------------------------------------------------------------------------------------*/
}

/*--------------------------------------------------------------------------------------------------------------------*/

static void read_data(nyx_stack_s::tcp_client_s &client)
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
        size_t new_capa = max(required, client.recv_capa + RECV_BUFF_SIZE);

        str_t new_buff = static_cast<str_t>(nyx_memory_realloc(client.recv_buff, new_capa));

        client.recv_buff = new_buff;
        client.recv_capa = new_capa;
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    client.recv_size += static_cast<size_t>(client.tcp_client.read(static_cast<uint8_t *>(client.recv_buff) + client.recv_size, available));

    /*----------------------------------------------------------------------------------------------------------------*/
}

/*--------------------------------------------------------------------------------------------------------------------*/

static void consume_data(nyx_node_t *node, nyx_stack_s::tcp_client_s &client)
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

            client.recv_size = client.recv_size - consumed;

            memmove(client.recv_buff, static_cast<uint8_t *>(client.recv_buff) + consumed, client.recv_size);

            /*--------------------------------------------------------------------------------------------------------*/
            /* ADJUST BUFFER                                                                                          */
            /*--------------------------------------------------------------------------------------------------------*/

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
        /* REGISTER CLIENTS                                                                                           */
        /*------------------------------------------------------------------------------------------------------------*/

        WiFiClient new_client = stack->tcp_server.accept();

        if(new_client && new_client.connected())
        {
            for(int i = 0; i < MAX_TCP_CLIENTS; ++i)
            {
                auto &client = clients[i];

                if(!client.tcp_client || !client.tcp_client.connected())
                {
                    client.tcp_client = new_client;

                    client.recv_size = 0;
                    client.recv_capa = 0;

                    goto __ok;
                }
            }

            new_client.stop();
        }

        /*------------------------------------------------------------------------------------------------------------*/
        /* RECIEVE DATA                                                                                               */
        /*------------------------------------------------------------------------------------------------------------*/
__ok:
        for(int i = 0; i < MAX_TCP_CLIENTS; i++)
        {
            auto &client = clients[i];

            if(client.tcp_client && client.tcp_client.connected())
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

        if(!stack->mqtt_client.connected())
        {
            if(stack->mqtt_client.connect(node->node_id.buf, stack->mqtt_username, stack->mqtt_password))
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

        stack->mqtt_client.loop();

        /*------------------------------------------------------------------------------------------------------------*/
    }

    /*----------------------------------------------------------------------------------------------------------------*/
}

/*--------------------------------------------------------------------------------------------------------------------*/
#endif
/*--------------------------------------------------------------------------------------------------------------------*/
