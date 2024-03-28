/*--------------------------------------------------------------------------------------------------------------------*/

#include "mongoose/mongoose.h"

#include "indi_base_internal.h"

/*--------------------------------------------------------------------------------------------------------------------*/
/* HELPERS                                                                                                            */
/*--------------------------------------------------------------------------------------------------------------------*/

static bool mg_startswith(struct mg_str topic, struct mg_str prefix)
{
    return topic.len >= prefix.len && memcmp(topic.ptr, prefix.ptr, prefix.len) == 0;
}

/*--------------------------------------------------------------------------------------------------------------------*/

static void mqtt_pub(struct mg_connection *connection, struct mg_str topic, struct mg_str message, int qos, bool retain)
{
    struct mg_mqtt_opts opts;

    memset(&opts, 0, sizeof(struct mg_mqtt_opts));

    opts.topic = topic;
    opts.message = message;
    opts.qos = qos;
    opts.retain = retain;

    mg_mqtt_pub(connection, &opts);
}

/*--------------------------------------------------------------------------------------------------------------------*/

static void mqtt_sub(struct mg_connection *connection, struct mg_str topic, int qos)
{
    struct mg_mqtt_opts opts;

    memset(&opts, 0, sizeof(struct mg_mqtt_opts));

    opts.topic = topic;
    ////.message = message;
    opts.qos = qos;
    ////.retain = retain;

    mg_mqtt_sub(connection, &opts);
}

/*--------------------------------------------------------------------------------------------------------------------*/
/* SERVER                                                                                                             */
/*--------------------------------------------------------------------------------------------------------------------*/

struct ctx_s
{
    STR_t url;

    struct mg_mgr mgr;

    struct mg_mqtt_opts opts;

    struct mg_connection *connection;

    /**/

    indi_list_t *driver_list;

    indi_dict_t **vector_list;
};

/*--------------------------------------------------------------------------------------------------------------------*/

static int volatile s_signo = 0;

static void signal_handler(int signo)
{
    s_signo = signo;
}

/*--------------------------------------------------------------------------------------------------------------------*/

static struct mg_str SPECIAL_TOPICS[] = {
    MG_C_STR("indi/get_clients"),
    MG_C_STR("indi/get_drivers"),
    MG_C_STR("indi/get_properties"),
    MG_C_STR("indi/enable_blob"),
    MG_C_STR("indi"),
};

/*--------------------------------------------------------------------------------------------------------------------*/

static void mqtt_fn(struct mg_connection *connection, int ev, void *ev_data)
{
    struct ctx_s *ctx = (struct ctx_s *) connection->fn_data;

    /**/ if(ev == MG_EV_OPEN)
    {
        MG_INFO(("%lu OPEN", connection->id));
    }
    else if(ev == MG_EV_CONNECT)
    {
        MG_INFO(("%lu CONNECT", connection->id));
    }
    else if(ev == MG_EV_ERROR)
    {
        MG_ERROR(("%lu ERROR %s", connection->id, (char *) ev_data));
    }
    else if(ev == MG_EV_CLOSE)
    {
        ((struct ctx_s *) connection->fn_data)->connection = NULL;
    }
    else if(ev == MG_EV_MQTT_OPEN)
    {
        /*------------------------------------------------------------------------------------------------------------*/
        /* MG_EV_MQTT_OPEN                                                                                            */
        /*------------------------------------------------------------------------------------------------------------*/

        MG_INFO(("%lu MQTT OPEN", connection->id));

        /*------------------------------------------------------------------------------------------------------------*/

        mqtt_pub(connection, mg_str("indi/status"), ctx->opts.client_id, 1, false);

        /*------------------------------------------------------------------------------------------------------------*/

        for(int i = 0; i < sizeof(SPECIAL_TOPICS) / sizeof(struct mg_str); i++)
        {
            str_t topic = indi_memory_alloc(SPECIAL_TOPICS[i].len + ctx->opts.client_id.len + 2);

            if(sprintf(topic, "%s/%s", SPECIAL_TOPICS[i].ptr, ctx->opts.client_id.ptr) > 0)
            {
                MG_INFO(("%lu Subscribing %s", connection->id, SPECIAL_TOPICS[i]));
                mqtt_sub(connection, SPECIAL_TOPICS[i], 1);

                MG_INFO(("%lu Subscribing %s", connection->id, mg_str(topic)));
                mqtt_sub(connection, mg_str(topic), 1);
            }

            indi_memory_free(topic);
        }

        /*------------------------------------------------------------------------------------------------------------*/
    }
    else if(ev == MG_EV_MQTT_MSG)
    {
        /*------------------------------------------------------------------------------------------------------------*/
        /* MG_EV_MQTT_MSG                                                                                             */
        /*------------------------------------------------------------------------------------------------------------*/

        struct mg_mqtt_message *message = (struct mg_mqtt_message *) ev_data;

        if(message->topic.len > 0 && message->topic.ptr != NULL
           &&
           message->data.len > 0 && message->data.ptr != NULL
        ) {
            /**/ if(mg_startswith(message->topic, SPECIAL_TOPICS[0]))
            {
                /*----------------------------------------------------------------------------------------------------*/
                /* GET_CLIENTS                                                                                        */
                /*----------------------------------------------------------------------------------------------------*/

                mqtt_pub(connection, mg_str("indi/clients"), ctx->opts.client_id, 1, false);

                /*----------------------------------------------------------------------------------------------------*/
            }
            else if(mg_startswith(message->topic, SPECIAL_TOPICS[1]))
            {
                /*----------------------------------------------------------------------------------------------------*/
                /* GET_DRIVERS                                                                                        */
                /*----------------------------------------------------------------------------------------------------*/

                str_t json = indi_list_to_string(ctx->driver_list);

                mqtt_pub(connection, mg_str("indi/drivers"), mg_str(json), 1, false);

                indi_memory_free(json);

                /*----------------------------------------------------------------------------------------------------*/
            }
            else if(mg_startswith(message->topic, SPECIAL_TOPICS[2]))
            {
                /*----------------------------------------------------------------------------------------------------*/
                /* GET_PROPERTIES                                                                                     */
                /*----------------------------------------------------------------------------------------------------*/

                for(indi_dict_t **vector_ptr = ctx->vector_list; *vector_ptr != NULL; vector_ptr++)
                {
                    str_t json = indi_dict_to_string(*vector_ptr);

                    mqtt_pub(connection, ctx->opts.client_id, mg_str(json), 1, false);

                    indi_memory_free(json);
                }

                /*----------------------------------------------------------------------------------------------------*/
            }
            else if(mg_startswith(message->topic, SPECIAL_TOPICS[3]))
            {
                /*----------------------------------------------------------------------------------------------------*/
                /* GET_PROPERTIES                                                                                     */
                /*----------------------------------------------------------------------------------------------------*/

                MG_INFO((">> TODO enable_blob"));

                /*----------------------------------------------------------------------------------------------------*/
            }
            else if(mg_startswith(message->topic, SPECIAL_TOPICS[4]))
            {
                /*----------------------------------------------------------------------------------------------------*/
                /* NEW XXX VECTOR                                                                                     */
                /*----------------------------------------------------------------------------------------------------*/

                MG_INFO((">> TODO treating: %.*s", (int) message->data.len, message->data.ptr));

                /*----------------------------------------------------------------------------------------------------*/
            }
        }

        /*------------------------------------------------------------------------------------------------------------*/
        /*------------------------------------------------------------------------------------------------------------*/
    }
}

/*--------------------------------------------------------------------------------------------------------------------*/

static void timer_fn(void *arg)
{
    struct ctx_s *ctx = (struct ctx_s *) arg;

    if(ctx->connection == NULL)
    {
        ctx->connection = mg_mqtt_connect(&ctx->mgr, ctx->url, &ctx->opts, mqtt_fn, ctx);
    }
}

/*--------------------------------------------------------------------------------------------------------------------*/

int indi_run(STR_t url, __NULLABLE__ STR_t username, __NULLABLE__ STR_t password, STR_t client_id, indi_list_t *driver_list, indi_dict_t *vector_list[])
{
    /*----------------------------------------------------------------------------------------------------------------*/

    struct ctx_s ctx;

    memset(&ctx, 0, sizeof(struct ctx_s));

    /*----------------------------------------------------------------------------------------------------------------*/

    /* SERVER */

    ctx.url = url;

    ctx.opts.user = mg_str(username);
    ctx.opts.pass = mg_str(password);

    ctx.driver_list = driver_list;
    ctx.vector_list = vector_list;

    /*----------------------------------------------------------------------------------------------------------------*/

    /* CLIENT */

    ctx.opts.clean = true;

    ctx.opts.version = 0x04;

    ctx.opts.client_id = mg_str(client_id);

    /*----------------------------------------------------------------------------------------------------------------*/

    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    /*----------------------------------------------------------------------------------------------------------------*/

    mg_mgr_init(&ctx.mgr);

    mg_timer_add(&ctx.mgr, 3000, MG_TIMER_REPEAT | MG_TIMER_RUN_NOW, timer_fn, &ctx);

    while(s_signo == 0) mg_mgr_poll(&ctx.mgr, 1000);

    mg_mgr_free(&ctx.mgr);

    /*----------------------------------------------------------------------------------------------------------------*/

    return 0;
}

/*--------------------------------------------------------------------------------------------------------------------*/
