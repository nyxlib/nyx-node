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

#pragma clang diagnostic push
#pragma ide diagnostic ignored "ConstantParameter"
static void mqtt_pub(struct mg_connection *connection, struct mg_str topic, struct mg_str message, int qos, bool retain)
#pragma clang diagnostic pop
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

#pragma clang diagnostic push
#pragma ide diagnostic ignored "ConstantParameter"
static void mqtt_sub(struct mg_connection *connection, struct mg_str topic, int qos)
#pragma clang diagnostic pop
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

struct indi_server_ctx_s
{
    STR_t url;

    struct mg_mgr mgr;
    struct mg_mqtt_opts opts;
    struct mg_connection *connection;

    /**/

    bool emit_xml;
    bool validate_xml;

    /**/

    indi_blob_t blob;

    /**/

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
    MG_C_STR("indi/cmd/get_clients"),
    MG_C_STR("indi/cmd/get_properties"),
    MG_C_STR("indi/cmd/enable_blob"),
    MG_C_STR("indi/cmd/json"),
    MG_C_STR("indi/cmd/xml"),
};

/*--------------------------------------------------------------------------------------------------------------------*/

static void out_callback(const indi_object_t *object)
{
    struct indi_server_ctx_s *ctx = (struct indi_server_ctx_s *) object->server_ctx;

    indi_object_t *string = indi_dict_get((indi_dict_t *) object, "<>");

    if(string != NULL && string->type == INDI_TYPE_STRING)
    {
        /*------------------------------------------------------------------------------------------------------------*/

        STR_t tag = indi_string_get((indi_string_t *) string);

        /*------------------------------------------------------------------------------------------------------------*/

        indi_dict_t *dict;

        /**/ if(strcmp("defNumberVector", tag) == 0) {
            dict = indi_number_set_vector_new((indi_dict_t *) object);
        }
        else if(strcmp("defTextVector", tag) == 0) {
            dict = indi_text_set_vector_new((indi_dict_t *) object);
        }
        else if(strcmp("defLightVector", tag) == 0) {
            dict = indi_light_set_vector_new((indi_dict_t *) object);
        }
        else if(strcmp("defSwitchVector", tag) == 0) {
            dict = indi_switch_set_vector_new((indi_dict_t *) object);
        }
        else if(strcmp("defBLOBVector", tag) == 0) {
            dict = indi_blob_set_vector_new((indi_dict_t *) object);
        }
        else {
            return;
        }

        /*------------------------------------------------------------------------------------------------------------*/

        if(ctx->emit_xml)
        {
            indi_xmldoc_t *xmldoc = indi_object_to_xmldoc(object, ctx->validate_xml);

            if(xmldoc != NULL)
            {
                /*----------------------------------------------------------------------------------------------------*/

                str_t xml = indi_xmldoc_to_string(xmldoc);
                mqtt_pub(ctx->connection, mg_str("indi/xml"), mg_str(xml), 1, false);
                indi_memory_free(xml);

                indi_xmldoc_free(xmldoc);

                /*----------------------------------------------------------------------------------------------------*/
            }
        }

        /*------------------------------------------------------------------------------------------------------------*/

        str_t json = indi_dict_to_string(dict);
        mqtt_pub(ctx->connection, mg_str("indi/json"), mg_str(json), 1, false);
        indi_memory_free(json);

        indi_dict_free(dict);

        /*------------------------------------------------------------------------------------------------------------*/
    }
}

/*--------------------------------------------------------------------------------------------------------------------*/

static void update_props(unsigned long id, indi_dict_t *vector_list[], const indi_dict_t *dict)
{
    /*----------------------------------------------------------------------------------------------------------------*/

    indi_object_t *device1_string = indi_dict_get(dict, "@device");
    indi_object_t *name1_string = indi_dict_get(dict, "@name");
    indi_object_t *children1_list = indi_dict_get(dict, "children");

    if(device1_string != NULL && device1_string->type == INDI_TYPE_STRING
       &&
       name1_string != NULL && name1_string->type == INDI_TYPE_STRING
       &&
       children1_list != NULL && children1_list->type == INDI_TYPE_LIST
    ) {
        STR_t device1 = indi_string_get((indi_string_t *) device1_string);
        STR_t name1 = indi_string_get((indi_string_t *) name1_string);

        /*------------------------------------------------------------------------------------------------------------*/

        for(indi_dict_t **vector_ptr = vector_list; *vector_ptr != NULL; vector_ptr++)
        {
            indi_dict_t *vector = *vector_ptr;

            /*--------------------------------------------------------------------------------------------------------*/

            indi_object_t *device2_string = indi_dict_get(vector, "@device");
            indi_object_t *name2_string = indi_dict_get(vector, "@name");
            indi_object_t *children2_list = indi_dict_get(vector, "children");

            if(device2_string != NULL && device2_string->type == INDI_TYPE_STRING
               &&
               name2_string != NULL && name2_string->type == INDI_TYPE_STRING
               &&
               children2_list != NULL && children1_list->type == INDI_TYPE_LIST
            ) {
                STR_t device2 = indi_string_get((indi_string_t *) device2_string);
                STR_t name2 = indi_string_get((indi_string_t *) name2_string);

                /*----------------------------------------------------------------------------------------------------*/

                if(strcmp(device1, device2) == 0 && strcmp(name1, name2) == 0)
                {
                    int idx1;
                    int idx2;

                    indi_object_t *object1;
                    indi_object_t *object2;

                    /*------------------------------------------------------------------------------------------------*/

                    for(indi_list_iter_t iter1 = INDI_LIST_ITER(children1_list); indi_list_iterate(&iter1, &idx1, &object1);)
                    {
                        if(object1->type == INDI_TYPE_DICT)
                        {
                            indi_object_t *prop1_string = indi_dict_get((indi_dict_t *) object1, "@name");

                            if(prop1_string != NULL && prop1_string->type == INDI_TYPE_STRING)
                            {
                                STR_t prop1 = indi_string_get((indi_string_t *) prop1_string);

                                /*------------------------------------------------------------------------------------*/

                                for(indi_list_iter_t iter2 = INDI_LIST_ITER(children2_list); indi_list_iterate(&iter2, &idx2, &object2);)
                                {
                                    if(object2->type == INDI_TYPE_DICT)
                                    {
                                        indi_object_t *prop2_string = indi_dict_get((indi_dict_t *) object2, "@name");

                                        if(prop2_string != NULL && prop2_string->type == INDI_TYPE_STRING)
                                        {
                                            STR_t prop2 = indi_string_get((indi_string_t *) prop2_string);

                                            /*------------------------------------------------------------------------*/

                                            if(strcmp(prop1, prop2) == 0)
                                            {
                                                bool notify = internal_copy_entry(
                                                    (indi_dict_t *) object2,
                                                    (indi_dict_t *) object1,
                                                    "$"
                                                );

                                                if(notify)
                                                {
                                                    str_t str = indi_object_to_string(object2);
                                                    MG_INFO(("%lu Updating `%s::%s` with %s", id, device1, name1, str));
                                                    indi_memory_free(str);

                                                    if(object2->in_callback != NULL)
                                                    {
                                                        object2->in_callback(object2);
                                                    }
                                                }
                                            }

                                            /*------------------------------------------------------------------------*/
                                        }
                                    }
                                }

                                /*------------------------------------------------------------------------------------*/
                            }
                        }
                    }

                    /*------------------------------------------------------------------------------------------------*/

                    break;
                }

                /*----------------------------------------------------------------------------------------------------*/
            }

            /*--------------------------------------------------------------------------------------------------------*/
        }

        /*------------------------------------------------------------------------------------------------------------*/
    }

    /*----------------------------------------------------------------------------------------------------------------*/
}

/*--------------------------------------------------------------------------------------------------------------------*/

static void mqtt_fn(struct mg_connection *connection, int ev, void *ev_data)
{
    struct indi_server_ctx_s *ctx = (struct indi_server_ctx_s *) connection->fn_data;

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
        ((struct indi_server_ctx_s *) connection->fn_data)->connection = NULL;
    }
    else if(ev == MG_EV_MQTT_OPEN)
    {
        /*------------------------------------------------------------------------------------------------------------*/
        /* MG_EV_MQTT_OPEN                                                                                            */
        /*------------------------------------------------------------------------------------------------------------*/

        MG_INFO(("%lu MQTT OPEN", connection->id));

        /*------------------------------------------------------------------------------------------------------------*/

        for(int i = 0; i < sizeof(SPECIAL_TOPICS) / sizeof(struct mg_str); i++)
        {
            str_t topic = indi_memory_alloc(SPECIAL_TOPICS[i].len + ctx->opts.client_id.len + 2);

            if(sprintf(topic, "%s/%s", SPECIAL_TOPICS[i].ptr, ctx->opts.client_id.ptr) > 0)
            {
                MG_INFO(("%lu Subscribing to `%s` and `%s` topics",
                    connection->id,
                    SPECIAL_TOPICS[i].ptr,
                    /*-----*/ topic /*-----*/
                ));

                mqtt_sub(connection, SPECIAL_TOPICS[i], 1);

                mqtt_sub(connection, mg_str(topic), 1);
            }

            indi_memory_free(topic);
        }

        /*------------------------------------------------------------------------------------------------------------*/

        mqtt_pub(connection, mg_str("indi/cmd/get_properties"), mg_str("1.7"), 1, false);

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
                /* GET_PROPERTIES                                                                                     */
                /*----------------------------------------------------------------------------------------------------*/

                for(indi_dict_t **vector_ptr = ctx->vector_list; *vector_ptr != NULL; vector_ptr++)
                {
                    str_t json = indi_dict_to_string(*vector_ptr);

                    mqtt_pub(connection, mg_str("indi/json"), mg_str(json), 1, false);

                    indi_memory_free(json);
                }

                /*----------------------------------------------------------------------------------------------------*/
            }
            else if(mg_startswith(message->topic, SPECIAL_TOPICS[2]))
            {
                /*----------------------------------------------------------------------------------------------------*/
                /* ENABLE_BLOB                                                                                        */
                /*----------------------------------------------------------------------------------------------------*/

                MG_INFO((">> TODO enable_blob"));

                /*----------------------------------------------------------------------------------------------------*/
            }
            else if(mg_startswith(message->topic, SPECIAL_TOPICS[3]))
            {
                /*----------------------------------------------------------------------------------------------------*/
                /* JSON NEW XXX VECTOR                                                                                */
                /*----------------------------------------------------------------------------------------------------*/

                indi_object_t *object = indi_object_parse(message->data.ptr);

                if(object != NULL)
                {
                    if(object->type == INDI_TYPE_DICT)
                    {
                        update_props(connection->id, ctx->vector_list, (indi_dict_t *) object);
                    }

                    indi_dict_free((indi_dict_t *) object);
                }

                /*----------------------------------------------------------------------------------------------------*/
            }
            else if(mg_startswith(message->topic, SPECIAL_TOPICS[4]))
            {
                /*----------------------------------------------------------------------------------------------------*/
                /* XML NEW XXX VECTOR                                                                                 */
                /*----------------------------------------------------------------------------------------------------*/

                indi_xmldoc_t *xmldoc = indi_xmldoc_parse(message->data.ptr);

                if(xmldoc != NULL)
                {
                    indi_object_t *object = indi_xmldoc_to_object(xmldoc, ctx->validate_xml);

                    if(object != NULL)
                    {
                        if(object->type == INDI_TYPE_DICT)
                        {
                            update_props(connection->id, ctx->vector_list, (indi_dict_t *) object);
                        }

                        indi_dict_free((indi_dict_t *) object);
                    }

                    indi_xmldoc_free(xmldoc);
                }

                /*----------------------------------------------------------------------------------------------------*/
            }
        }

        /*------------------------------------------------------------------------------------------------------------*/
    }
}

/*--------------------------------------------------------------------------------------------------------------------*/

static void timer_fn(void *arg)
{
    struct indi_server_ctx_s *ctx = (struct indi_server_ctx_s *) arg;

    if(ctx->connection == NULL)
    {
        ctx->connection = mg_mqtt_connect(&ctx->mgr, ctx->url, &ctx->opts, mqtt_fn, ctx);
    }
}

/*--------------------------------------------------------------------------------------------------------------------*/

int indi_run(STR_t url, __NULLABLE__ STR_t username, __NULLABLE__ STR_t password, STR_t client_id, indi_dict_t *vector_list[], bool emit_xml, bool validate_xml)
{
    /*----------------------------------------------------------------------------------------------------------------*/

    struct indi_server_ctx_s ctx;

    memset(&ctx, 0, sizeof(struct indi_server_ctx_s));

    /*----------------------------------------------------------------------------------------------------------------*/

    ctx.url = url;

    ctx.opts.user = mg_str(username);
    ctx.opts.pass = mg_str(password);

    ctx.opts.clean = true;
    ctx.opts.version = 0x04;

    ctx.opts.client_id = mg_str(client_id);

    /*----------------------------------------------------------------------------------------------------------------*/

    ctx.emit_xml = emit_xml;
    ctx.validate_xml = validate_xml;

    /*----------------------------------------------------------------------------------------------------------------*/

    ctx.blob = INDI_BLOB_NEVER;

    /*----------------------------------------------------------------------------------------------------------------*/

    ctx.vector_list = vector_list;

    /*----------------------------------------------------------------------------------------------------------------*/

    for(indi_dict_t **vector_ptr = vector_list; *vector_ptr != NULL; vector_ptr++)
    {
        indi_dict_set(*vector_ptr, "@client", indi_string_from(client_id));

        (*vector_ptr)->base.out_callback = &out_callback;

        (*vector_ptr)->base.server_ctx = &ctx;
    }

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
