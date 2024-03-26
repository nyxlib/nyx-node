/*--------------------------------------------------------------------------------------------------------------------*/

#include "mongoose/mongoose.h"

#include "indi_base_internal.h"

/*--------------------------------------------------------------------------------------------------------------------*/

struct ctx_s
{
    STR_t url;

    struct mg_mgr mgr;

    struct mg_mqtt_opts opts;

    struct mg_connection *connection;
};

/*--------------------------------------------------------------------------------------------------------------------*/

static int volatile s_signo = 0;

static void signal_handler(int signo)
{
    s_signo = signo;
}

/*--------------------------------------------------------------------------------------------------------------------*/
/* HELPERS                                                                                                            */
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
        MG_INFO(("%lu MQTT OPEN", connection->id));

        /*------------------------------------------------------------------------------------------------------------*/

        mqtt_pub(connection, mg_str("indi/status"), ctx->opts.client_id, 1, false);

        /*------------------------------------------------------------------------------------------------------------*/

        static STR_t topics[] = {
            "indi/get_properties",
            "indi/get_clients",
            "indi/get_drivers"
        };

        for(int i = 0; i < sizeof(topics) / sizeof(STR_t); i++)
        {
            str_t topic = indi_memory_alloc(strlen(topics[i]) + ctx->opts.client_id.len + 2);

            sprintf(topic, "%s/%s", topics[i], ctx->opts.client_id.ptr);

            mqtt_sub(connection, mg_str(topics[i]), 1);

            mqtt_sub(connection, mg_str(topic), 1);

            indi_memory_free(topic);
        }

        /*------------------------------------------------------------------------------------------------------------*/
    }
    else if(ev == MG_EV_MQTT_MSG)
    {
        struct mg_mqtt_message *message = (struct mg_mqtt_message *) ev_data;

        if(message->topic.len > 0 && message->topic.ptr != NULL
           &&
           message->data.len > 0 && message->data.ptr != NULL
        ) {
            MG_INFO(("%lu MQTT MSG %.*s <- %.*s", connection->id, (int) message->data.len, message->data.ptr, (int) message->topic.len, message->topic.ptr));
        }
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

int indi_run(STR_t url, /* nullable */ STR_t username, /* nullable */ STR_t password, STR_t client_id)
{
    /*----------------------------------------------------------------------------------------------------------------*/

    struct ctx_s ctx;

    memset(&ctx, 0, sizeof(struct ctx_s));

    /* SERVER */

    ctx.url = url;

    ctx.opts.user = mg_str(username);
    ctx.opts.pass = mg_str(password);

    /* CLIENT */

    ctx.opts.clean = true;

    ctx.opts.version = 0x04;

    ctx.opts.client_id = mg_str(client_id);

    /*----------------------------------------------------------------------------------------------------------------*/

    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    mg_mgr_init(&ctx.mgr);

    mg_timer_add(&ctx.mgr, 3000, MG_TIMER_REPEAT | MG_TIMER_RUN_NOW, timer_fn, &ctx);

    while(s_signo == 0) mg_mgr_poll(&ctx.mgr, 1000);

    mg_mgr_free(&ctx.mgr);

    /*----------------------------------------------------------------------------------------------------------------*/

    return 0;
}

/*--------------------------------------------------------------------------------------------------------------------*/
