/*--------------------------------------------------------------------------------------------------------------------*/

#include "mongoose/mongoose.h"

#include "indi_node_internal.h"

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

    memset(&opts, 0x00, sizeof(struct mg_mqtt_opts));

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

    memset(&opts, 0x00, sizeof(struct mg_mqtt_opts));

    opts.topic = topic;
    ////.message = message;
    opts.qos = qos;
    ////.retain = retain;

    mg_mqtt_sub(connection, &opts);
}

/*--------------------------------------------------------------------------------------------------------------------*/
/* SERVER                                                                                                             */
/*--------------------------------------------------------------------------------------------------------------------*/

struct indi_node_s
{
    STR_t tcp_url;
    STR_t mqtt_url;

    struct mg_mgr mgr;
    struct mg_mqtt_opts mqtt_opts;
    struct mg_connection *tcp_connection;
    struct mg_connection *mqtt_connection;

    /**/

    indi_dict_t **def_vectors;

    /**/

    bool enable_xml;
    bool validate_xml;
};

/*--------------------------------------------------------------------------------------------------------------------*/

static struct mg_str SPECIAL_TOPICS[] = {
    MG_C_STR("indi/cmd/get_clients"),
    MG_C_STR("indi/cmd/json"),
    MG_C_STR("indi/cmd/xml"),
};

/*--------------------------------------------------------------------------------------------------------------------*/

static void tcp_sub(struct indi_node_s *node, STR_t message)
{
    for(struct mg_connection *connection = node->mgr.conns; connection != NULL; connection = connection->next)
    {
        if(connection != node->tcp_connection
           &&
           connection != node->mqtt_connection
        ) {
            mg_send(connection, message, strlen(message));
        }
    }
}

/*--------------------------------------------------------------------------------------------------------------------*/

static void sub_object(struct indi_node_s *node, indi_object_t *object)
{
    /*----------------------------------------------------------------------------------------------------------------*/

    if(node->enable_xml)
    {
        indi_xmldoc_t *xmldoc = indi_object_to_xmldoc(object, node->validate_xml);

        if(xmldoc != NULL)
        {
            /*--------------------------------------------------------------------------------------------------------*/

            str_t xml = indi_xmldoc_to_string(xmldoc);
            mqtt_pub(node->mqtt_connection, mg_str("indi/xml"), mg_str(xml), 1, false);
            tcp_sub(node, xml);
            indi_memory_free(xml);

            /*--------------------------------------------------------------------------------------------------------*/

            indi_xmldoc_free(xmldoc);

            /*--------------------------------------------------------------------------------------------------------*/
        }
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    str_t json = indi_object_to_string(object);
    mqtt_pub(node->mqtt_connection, mg_str("indi/json"), mg_str(json), 1, false);
    ////_sub(node, json);
    indi_memory_free(json);

    /*----------------------------------------------------------------------------------------------------------------*/
}

/*--------------------------------------------------------------------------------------------------------------------*/

static void out_callback(indi_object_t *object)
{
    const indi_dict_t *def_vector = (indi_dict_t *) object;

    STR_t tag_name = indi_dict_get_string(def_vector, "<>");

    if(tag_name != NULL)
    {
        /*------------------------------------------------------------------------------------------------------------*/

        indi_dict_t *set_vector;

        /**/ if(strcmp("defNumberVector", tag_name) == 0) {
            set_vector = indi_number_set_vector_new(def_vector);
        }
        else if(strcmp("defTextVector", tag_name) == 0) {
            set_vector = indi_text_set_vector_new(def_vector);
        }
        else if(strcmp("defLightVector", tag_name) == 0) {
            set_vector = indi_light_set_vector_new(def_vector);
        }
        else if(strcmp("defSwitchVector", tag_name) == 0) {
            set_vector = indi_switch_set_vector_new(def_vector);
        }
        else if(strcmp("defBLOBVector", tag_name) == 0) {
            set_vector = indi_blob_set_vector_new(def_vector);
        }
        else {
            return;
        }

        /*------------------------------------------------------------------------------------------------------------*/

        sub_object(object->node, (indi_object_t *) set_vector);

        /*------------------------------------------------------------------------------------------------------------*/

        indi_dict_free(set_vector);

        /*------------------------------------------------------------------------------------------------------------*/
    }
}

/*--------------------------------------------------------------------------------------------------------------------*/

static void get_properties(indi_node_t *node, const indi_dict_t *dict)
{
    /*----------------------------------------------------------------------------------------------------------------*/

    STR_t device1 = indi_dict_get_string(dict, "@device");
    STR_t name1 = indi_dict_get_string(dict, "@name");

    /*----------------------------------------------------------------------------------------------------------------*/

    for(indi_dict_t **def_vector_ptr = node->def_vectors; *def_vector_ptr != NULL; def_vector_ptr++)
    {
        indi_dict_t *def_vector = *def_vector_ptr;

        /*------------------------------------------------------------------------------------------------------------*/

        STR_t device2 = indi_dict_get_string(def_vector, "@device");
        STR_t name2 = indi_dict_get_string(def_vector, "@name");

        /*------------------------------------------------------------------------------------------------------------*/

        if(device1 != NULL)
        {
            if((device2 == NULL || strcmp(device1, device2) != 0))
            {
                break;
            }

            if(name1 != NULL)
            {
                if((name2 == NULL || strcmp(name1, name2) != 0))
                {
                    break;
                }
            }
        }

        /*------------------------------------------------------------------------------------------------------------*/

        sub_object(node, (indi_object_t *) def_vector);

        /*------------------------------------------------------------------------------------------------------------*/
    }
}

/*--------------------------------------------------------------------------------------------------------------------*/

static void enable_blob(indi_node_t *node, const indi_dict_t *dict)
{
    STR_t device = indi_dict_get_string(dict, "@device");
    STR_t name = indi_dict_get_string(dict, "@name");
    STR_t val = indi_dict_get_string(dict, "$");

    if(device != NULL && val != NULL)
    {
        if(name != NULL)
        {
            MG_INFO(("%lu Enable blob set to `%s` for `%s::%s` ", node->mqtt_connection->id, val, device, name));
        }
        else
        {
            MG_INFO(("%lu Enable blob set to `%s` for `%s` ", node->mqtt_connection->id, val, device));
        }
    }
}

/*--------------------------------------------------------------------------------------------------------------------*/

static void update_props(indi_node_t *node, const indi_dict_t *dict)
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

        for(indi_dict_t **def_vector_ptr = node->def_vectors; *def_vector_ptr != NULL; def_vector_ptr++)
        {
            indi_dict_t *def_vector = *def_vector_ptr;

            /*--------------------------------------------------------------------------------------------------------*/

            indi_object_t *device2_string = indi_dict_get(def_vector, "@device");
            indi_object_t *name2_string = indi_dict_get(def_vector, "@name");
            indi_object_t *children2_list = indi_dict_get(def_vector, "children");

            if(device2_string != NULL && device2_string->type == INDI_TYPE_STRING
               &&
               name2_string != NULL && name2_string->type == INDI_TYPE_STRING
               &&
               children2_list != NULL && children1_list->type == INDI_TYPE_LIST
            ) {
                STR_t device2 = indi_string_get((indi_string_t *) device2_string);
                STR_t name2 = indi_string_get((indi_string_t *) name2_string);

                /*----------------------------------------------------------------------------------------------------*/

                if(strcmp(device1, device2) == 0
                   &&
                   strcmp(name1, name2) == 0
                ) {
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
                                                    MG_INFO(("%lu Updating `%s::%s` with %s", node->mqtt_connection->id, device1, name1, str));
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

                    /* property found */

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

static void process_message(indi_node_t *node, indi_object_t *object)
{
    if(object->type == INDI_TYPE_DICT)
    {
        STR_t tag_name = indi_dict_get_string((indi_dict_t *) object, "<>");

        if(tag_name != NULL)
        {
            /**/ if(strcmp(tag_name, "getProperties") == 0) {
                get_properties(node, (indi_dict_t *) object);
            }
            else if(strcmp(tag_name, "enableBLOB") == 0) {
                enable_blob(node, (indi_dict_t *) object);
            }
            else if(strcmp(tag_name, "newNumberVector") == 0
                    ||
                    strcmp(tag_name, "newTextVector") == 0
                    ||
                    strcmp(tag_name, "newLightVector") == 0
                    ||
                    strcmp(tag_name, "newSwitchVector") == 0
                    ||
                    strcmp(tag_name, "newBLOBVector") == 0
            ) {
                update_props(node, (indi_dict_t *) object);
            }
        }
    }
}

/*--------------------------------------------------------------------------------------------------------------------*/

static void tcp_handler(struct mg_connection *connection, int ev, void *ev_data)
{
    indi_node_t *node = (indi_node_t *) connection->fn_data;

    /**/ if(ev == MG_EV_OPEN)
    {
        MG_INFO(("%lu OPEN", connection->id));
    }
    else if(ev == MG_EV_ACCEPT)
    {
        MG_INFO(("%lu ACCEPT", connection->id));
    }
    else if(ev == MG_EV_CLOSE)
    {
        MG_INFO(("%lu CLOSE", connection->id));
    }
    else if(ev == MG_EV_ERROR)
    {
        MG_ERROR(("%lu ERROR %s", connection->id, (STR_t) ev_data));
    }
    else if(ev == MG_EV_READ)
    {
        /*------------------------------------------------------------------------------------------------------------*/
        /* MG_EV_READ                                                                                                 */
        /*------------------------------------------------------------------------------------------------------------*/

        indi_stream_t stream = INDI_STREAM();

        struct mg_iobuf *iobuf = &connection->recv;

        if(indi_stream_detect_opening_tag(&stream, iobuf->len, iobuf->buf))
        {
            if(indi_stream_detect_closing_tag(&stream, iobuf->len, iobuf->buf))
            {
                /*----------------------------------------------------------------------------------------------------*/

                indi_xmldoc_t *xmldoc = indi_xmldoc_parse_buff(stream.s_ptr, stream.len);

                if(xmldoc != NULL)
                {
                    indi_object_t *object = indi_xmldoc_to_object(xmldoc, node->validate_xml);

                    if(object != NULL)
                    {
                        process_message(node, object);

                        indi_object_free(object);
                    }

                    indi_xmldoc_free(xmldoc);
                }

                /*----------------------------------------------------------------------------------------------------*/

                mg_iobuf_del(iobuf, 0, stream.pos + stream.len);

                /*----------------------------------------------------------------------------------------------------*/
            }
        }

        /*------------------------------------------------------------------------------------------------------------*/
    }
}

/*--------------------------------------------------------------------------------------------------------------------*/

static void mqtt_handler(struct mg_connection *connection, int ev, void *ev_data)
{
    indi_node_t *node = (indi_node_t *) connection->fn_data;

    /**/ if(ev == MG_EV_OPEN)
    {
        MG_INFO(("%lu OPEN", connection->id));
    }
    else if(ev == MG_EV_CONNECT)
    {
        MG_INFO(("%lu CONNECT", connection->id));
    }
    else if(ev == MG_EV_CLOSE)
    {
        MG_INFO(("%lu CLOSE", connection->id));

        ((indi_node_t *) connection->fn_data)->mqtt_connection = NULL;
    }
    else if(ev == MG_EV_ERROR)
    {
        MG_ERROR(("%lu ERROR %s", connection->id, (STR_t) ev_data));
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
            str_t topic = indi_memory_alloc(SPECIAL_TOPICS[i].len + node->mqtt_opts.client_id.len + 2);

            if(sprintf(topic, "%s/%s", SPECIAL_TOPICS[i].ptr, node->mqtt_opts.client_id.ptr) > 0)
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

                mqtt_pub(connection, mg_str("indi/clients"), node->mqtt_opts.client_id, 1, false);

                /*----------------------------------------------------------------------------------------------------*/
            }
            else if(mg_startswith(message->topic, SPECIAL_TOPICS[1]))
            {
                /*----------------------------------------------------------------------------------------------------*/
                /* JSON NEW XXX VECTOR                                                                                */
                /*----------------------------------------------------------------------------------------------------*/

                indi_object_t *object = indi_object_parse(message->data.ptr);

                if(object != NULL)
                {
                    process_message(node, object);

                    indi_object_free(object);
                }

                /*----------------------------------------------------------------------------------------------------*/
            }
            else if(mg_startswith(message->topic, SPECIAL_TOPICS[2]))
            {
                /*----------------------------------------------------------------------------------------------------*/
                /* XML NEW XXX VECTOR                                                                                 */
                /*----------------------------------------------------------------------------------------------------*/

                indi_xmldoc_t *xmldoc = indi_xmldoc_parse(message->data.ptr);

                if(xmldoc != NULL)
                {
                    indi_object_t *object = indi_xmldoc_to_object(xmldoc, node->validate_xml);

                    if(object != NULL)
                    {
                        process_message(node, object);

                        indi_object_free(object);
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

static void timer_handle(void *arg)
{
    indi_node_t *node = (indi_node_t *) arg;

    if(node->mqtt_connection == NULL)
    {
        node->mqtt_connection = mg_mqtt_connect(&node->mgr, node->mqtt_url, &node->mqtt_opts, mqtt_handler, node);
    }
}

/*--------------------------------------------------------------------------------------------------------------------*/

indi_node_t *indi_node_init(
    __NULLABLE__ STR_t tcp_url,
    __NULLABLE__ STR_t mqtt_url,
    __NULLABLE__ STR_t username,
    __NULLABLE__ STR_t password,
    /**/
    STR_t node_id,
    indi_dict_t *def_vectors[],
    /**/
    int retry_ms,
    bool enable_xml,
    bool validate_xml
) {
    /*----------------------------------------------------------------------------------------------------------------*/

    indi_node_t *node = indi_memory_alloc(sizeof(indi_node_t));

    memset(node, 0x00, sizeof(indi_node_t));

    /*----------------------------------------------------------------------------------------------------------------*/
    /* PATH VECTORS                                                                                                   */
    /*----------------------------------------------------------------------------------------------------------------*/

    for(indi_dict_t **def_vector_ptr = def_vectors; *def_vector_ptr != NULL; def_vector_ptr++)
    {
        indi_dict_set(*def_vector_ptr, "@client", indi_string_from(node_id));

        (*def_vector_ptr)->base.out_callback = out_callback;

        (*def_vector_ptr)->base.node = node;
    }

    /*----------------------------------------------------------------------------------------------------------------*/
    /* SET NODE OPTIONS                                                                                               */
    /*----------------------------------------------------------------------------------------------------------------*/

    node->tcp_url = tcp_url;
    node->mqtt_url = mqtt_url;

    node->mqtt_opts.user = mg_str(username);
    node->mqtt_opts.pass = mg_str(password);

    node->mqtt_opts.clean = true;
    node->mqtt_opts.version = 0x04;

    node->mqtt_opts.client_id = mg_str(node_id);

    /*----------------------------------------------------------------------------------------------------------------*/

    node->def_vectors = def_vectors;

    node->enable_xml = enable_xml;

    node->validate_xml = validate_xml;

    /*----------------------------------------------------------------------------------------------------------------*/
    /* INITIALIZE TCP & MQTT CLIENTS                                                                                  */
    /*----------------------------------------------------------------------------------------------------------------*/

    mg_mgr_init(&node->mgr);

    /*----------------------------------------------------------------------------------------------------------------*/

    if(tcp_url != NULL)
    {
        mg_listen(&node->mgr, node->tcp_url, tcp_handler, node);
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    if(mqtt_url != NULL)
    {
        mg_timer_add(&node->mgr, retry_ms, MG_TIMER_REPEAT | MG_TIMER_RUN_NOW, timer_handle, node);
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    return node;
}

/*--------------------------------------------------------------------------------------------------------------------*/

void indi_node_pool(indi_node_t *node, int timeout_ms)
{
    mg_mgr_poll(&node->mgr, timeout_ms);
}

/*--------------------------------------------------------------------------------------------------------------------*/

void indi_node_free(indi_node_t *node, bool free_vectors)
{
    /*----------------------------------------------------------------------------------------------------------------*/

    mg_mgr_free(&node->mgr);

    /*----------------------------------------------------------------------------------------------------------------*/

    if(free_vectors)
    {
        for(indi_dict_t **def_vector_ptr = node->def_vectors; *def_vector_ptr != NULL; def_vector_ptr++)
        {
            indi_dict_free(*def_vector_ptr);
        }
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    indi_memory_free(node);

    /*----------------------------------------------------------------------------------------------------------------*/
}

/*--------------------------------------------------------------------------------------------------------------------*/
