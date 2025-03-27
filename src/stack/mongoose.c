/*--------------------------------------------------------------------------------------------------------------------*/
#if !defined(ARDUINO)
/*--------------------------------------------------------------------------------------------------------------------*/

#include <string.h>

#include "gen/mongoose.h"

#include "../nyx_node_internal.h"

/*--------------------------------------------------------------------------------------------------------------------*/

struct nyx_stack_s
{
    struct mg_mgr mgr;
    struct mg_mqtt_opts mqtt_opts;
    struct mg_connection *tcp_connection;
    struct mg_connection *mqtt_connection;
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

    mg_xprintf(nyx_log_func, nyx_log_args, "%s - %s:%d %s() - ", nyx_log_level_to_str(level), file, line, func);

    /*----------------------------------------------------------------------------------------------------------------*/
}

/*--------------------------------------------------------------------------------------------------------------------*/

void nyx_log(const char *fmt, ...)
{
    /*----------------------------------------------------------------------------------------------------------------*/

    va_list ap;
    va_start(ap, fmt);
    mg_vxprintf(nyx_log_func, nyx_log_args, fmt, &ap);
    va_end(ap);

    /*----------------------------------------------------------------------------------------------------------------*/

    nyx_log_func('\r', nyx_log_args);
    nyx_log_func('\n', nyx_log_args);

    /*----------------------------------------------------------------------------------------------------------------*/
}

/*--------------------------------------------------------------------------------------------------------------------*/
/* TCP & MQTT                                                                                                         */
/*--------------------------------------------------------------------------------------------------------------------*/

void nyx_tcp_pub(nyx_node_t *node, nyx_str_t message)
{
    for(struct mg_connection *connection = node->stack->mgr.conns; connection != NULL; connection = connection->next)
    {
        if(connection != node->stack->tcp_connection
           &&
           connection != node->stack->mqtt_connection
        ) {
            mg_send(connection, message.buf, message.len);
        }
    }
}

/*--------------------------------------------------------------------------------------------------------------------*/

void nyx_mqtt_sub(nyx_node_t *node, nyx_str_t topic)
{
    if(node->stack->mqtt_connection != NULL)
    {
        struct mg_mqtt_opts opts;

        memset(&opts, 0x00, sizeof(struct mg_mqtt_opts));

        opts.topic = topic;
        ////.message = message;
        opts.qos = 1;

        mg_mqtt_sub(node->stack->mqtt_connection, &opts);
    }
}

/*--------------------------------------------------------------------------------------------------------------------*/

void nyx_mqtt_pub(nyx_node_t *node, nyx_str_t topic, nyx_str_t message)
{
    if(node->stack->mqtt_connection != NULL)
    {
        struct mg_mqtt_opts opts;

        memset(&opts, 0x00, sizeof(struct mg_mqtt_opts));

        opts.topic = topic;
        opts.message = message;
        opts.qos = 1;

        mg_mqtt_pub(node->stack->mqtt_connection, &opts);
    }
}

/*--------------------------------------------------------------------------------------------------------------------*/
/* STACK                                                                                                              */
/*--------------------------------------------------------------------------------------------------------------------*/

static void tcp_handler(struct mg_connection *connection, int ev, void *ev_data)
{
    nyx_node_t *node = (nyx_node_t *) connection->fn_data;

    /**/ if(ev == MG_EV_OPEN)
    {
        NYX_INFO(("%lu OPEN", connection->id));
    }
    else if(ev == MG_EV_ACCEPT)
    {
        NYX_INFO(("%lu ACCEPT", connection->id));
    }
    else if(ev == MG_EV_CLOSE)
    {
        NYX_INFO(("%lu CLOSE", connection->id));
    }
    else if(ev == MG_EV_ERROR)
    {
        NYX_ERROR(("%lu ERROR %s", connection->id, (STR_t) ev_data));
    }
    else if(ev == MG_EV_READ)
    {
        /*------------------------------------------------------------------------------------------------------------*/
        /* MG_EV_READ                                                                                                 */
        /*------------------------------------------------------------------------------------------------------------*/

        size_t consumed = node->tcp_handler(node, NYX_EVENT_MSG, connection->recv.len, connection->recv.buf);

        mg_iobuf_del(
            &connection->recv,
            0x00000000,
            consumed
        );

        /*------------------------------------------------------------------------------------------------------------*/
    }
}

/*--------------------------------------------------------------------------------------------------------------------*/

static void mqtt_handler(struct mg_connection *connection, int ev, void *ev_data)
{
    nyx_node_t *node = (nyx_node_t *) connection->fn_data;

    /**/ if(ev == MG_EV_OPEN)
    {
        NYX_INFO(("%lu OPEN", connection->id));
    }
    else if(ev == MG_EV_CONNECT)
    {
        NYX_INFO(("%lu CONNECT", connection->id));
    }
    else if(ev == MG_EV_CLOSE)
    {
        NYX_INFO(("%lu CLOSE", connection->id));

        node->stack->mqtt_connection = NULL;
    }
    else if(ev == MG_EV_ERROR)
    {
        NYX_ERROR(("%lu ERROR %s", connection->id, (STR_t) ev_data));
    }
    else if(ev == MG_EV_MQTT_OPEN)
    {
        /*------------------------------------------------------------------------------------------------------------*/
        /* MG_EV_MQTT_OPEN                                                                                            */
        /*------------------------------------------------------------------------------------------------------------*/

        NYX_INFO(("%lu MQTT OPEN", connection->id));

        /*------------------------------------------------------------------------------------------------------------*/

        node->mqtt_handler(node, NYX_EVENT_OPEN, node->node_id, node->node_id);

        /*------------------------------------------------------------------------------------------------------------*/
    }
    else if(ev == MG_EV_MQTT_MSG)
    {
        /*------------------------------------------------------------------------------------------------------------*/
        /* MG_EV_MQTT_MSG                                                                                             */
        /*------------------------------------------------------------------------------------------------------------*/

        struct mg_mqtt_message *message = (struct mg_mqtt_message *) ev_data;

        node->mqtt_handler(node, NYX_EVENT_MSG, message->topic, message->data);

        /*------------------------------------------------------------------------------------------------------------*/
    }
}

/*--------------------------------------------------------------------------------------------------------------------*/

static void retry_timer_handler(void *arg)
{
    nyx_node_t *node = (nyx_node_t *) arg;

    nyx_stack_t *stack = (nyx_stack_t *) node->stack;

    if(stack->mqtt_connection == NULL)
    {
        stack->mqtt_connection = mg_mqtt_connect(
            &stack->mgr,
            node->mqtt_url,
            &stack->mqtt_opts,
            mqtt_handler,
            node
        );
    }
}

/*--------------------------------------------------------------------------------------------------------------------*/

static void ping_timer_handler(void *arg)
{
    nyx_node_t *node = (nyx_node_t *) arg;

    nyx_mqtt_pub(node, nyx_str_s("nyx/ping/node"), node->node_id);
}

/*--------------------------------------------------------------------------------------------------------------------*/

void nyx_node_stack_initialize(
    nyx_node_t *node,
    __NULLABLE__ STR_t mqtt_username,
    __NULLABLE__ STR_t mqtt_password,
    int retry_ms
) {
    /*----------------------------------------------------------------------------------------------------------------*/
    /* INITIALIZE LOGGER                                                                                              */
    /*----------------------------------------------------------------------------------------------------------------*/

    nyx_log_func = mg_pfn_stdout;

    /*----------------------------------------------------------------------------------------------------------------*/
    /* INITIALIZE STACK                                                                                               */
    /*----------------------------------------------------------------------------------------------------------------*/

    struct nyx_stack_s *stack = node->stack = nyx_memory_alloc(sizeof(struct nyx_stack_s));

    memset(stack, 0x00, sizeof(struct nyx_stack_s));

    /*----------------------------------------------------------------------------------------------------------------*/

    stack->mqtt_opts.user = mg_str(mqtt_username);
    stack->mqtt_opts.pass = mg_str(mqtt_password);

    stack->mqtt_opts.client_id = node->node_id;

    stack->mqtt_opts.clean = true;
    stack->mqtt_opts.version = 0x04;

    /*----------------------------------------------------------------------------------------------------------------*/

    mg_mgr_init(&stack->mgr);

    /*----------------------------------------------------------------------------------------------------------------*/

    if(node->tcp_url != NULL)
    {
        mg_listen(&stack->mgr, node->tcp_url, tcp_handler, node);
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    if(node->mqtt_url != NULL)
    {
        mg_timer_add(&stack->mgr, retry_ms, MG_TIMER_REPEAT | MG_TIMER_RUN_NOW, retry_timer_handler, node);

        mg_timer_add(&stack->mgr, NYX_PING_MS, MG_TIMER_REPEAT | MG_TIMER_RUN_NOW, ping_timer_handler, node);
    }

    /*----------------------------------------------------------------------------------------------------------------*/
}

/*--------------------------------------------------------------------------------------------------------------------*/

void nyx_node_stack_finalize(nyx_node_t *node)
{
    mg_mgr_free(&node->stack->mgr);

    nyx_memory_free(node->stack);
}

/*--------------------------------------------------------------------------------------------------------------------*/

void nyx_stack_poll(nyx_node_t *node, int timeout_ms)
{
    mg_mgr_poll(&node->stack->mgr, timeout_ms);
}

/*--------------------------------------------------------------------------------------------------------------------*/
#endif
/*--------------------------------------------------------------------------------------------------------------------*/
