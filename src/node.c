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
    STR_t url;

    struct mg_mgr mgr;
    struct mg_mqtt_opts opts;
    struct mg_connection *connection;

    /**/

    bool enable_xml;
    bool validate_xml;

    /**/

    indi_blob_t blob;

    /**/

    indi_dict_t **vector_list;
};

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
    struct indi_node_s *node = (struct indi_node_s *) object->node;

    indi_object_t *string = indi_dict_get((indi_dict_t *) object, "<>");

    if(string != NULL && string->type == INDI_TYPE_STRING)
    {
        /*------------------------------------------------------------------------------------------------------------*/

        STR_t tag = indi_string_get((indi_string_t *) string);

        /*------------------------------------------------------------------------------------------------------------*/

        bool is_blob;

        indi_dict_t *dict;

        /**/ if(strcmp("defNumberVector", tag) == 0) {
            is_blob = false;
            dict = indi_number_set_vector_new((indi_dict_t *) object);
        }
        else if(strcmp("defTextVector", tag) == 0) {
            is_blob = false;
            dict = indi_text_set_vector_new((indi_dict_t *) object);
        }
        else if(strcmp("defLightVector", tag) == 0) {
            is_blob = false;
            dict = indi_light_set_vector_new((indi_dict_t *) object);
        }
        else if(strcmp("defSwitchVector", tag) == 0) {
            is_blob = false;
            dict = indi_switch_set_vector_new((indi_dict_t *) object);
        }
        else if(strcmp("defBLOBVector", tag) == 0) {
            is_blob = true;
            dict = indi_blob_set_vector_new((indi_dict_t *) object);

        }
        else {
            return;
        }

        /*------------------------------------------------------------------------------------------------------------*/

        if((is_blob == true && node->blob == INDI_BLOB_NEVER)
           ||
           (is_blob == false && node->blob == INDI_BLOB_ONLY)
        ) {
            return;
        }

        /*------------------------------------------------------------------------------------------------------------*/

        if(node->enable_xml)
        {
            indi_xmldoc_t *xmldoc = indi_object_to_xmldoc(object, node->validate_xml);

            if(xmldoc != NULL)
            {
                /*----------------------------------------------------------------------------------------------------*/

                str_t xml = indi_xmldoc_to_string(xmldoc);
                mqtt_pub(node->connection, mg_str("indi/xml"), mg_str(xml), 1, false);
                indi_memory_free(xml);

                indi_xmldoc_free(xmldoc);

                /*----------------------------------------------------------------------------------------------------*/
            }
        }

        /*------------------------------------------------------------------------------------------------------------*/

        str_t json = indi_dict_to_string(dict);
        mqtt_pub(node->connection, mg_str("indi/json"), mg_str(json), 1, false);
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
    struct indi_node_s *node = (struct indi_node_s *) connection->fn_data;

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
        ((struct indi_node_s *) connection->fn_data)->connection = NULL;
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
            str_t topic = indi_memory_alloc(SPECIAL_TOPICS[i].len + node->opts.client_id.len + 2);

            if(sprintf(topic, "%s/%s", SPECIAL_TOPICS[i].ptr, node->opts.client_id.ptr) > 0)
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

                mqtt_pub(connection, mg_str("indi/clients"), node->opts.client_id, 1, false);

                /*----------------------------------------------------------------------------------------------------*/
            }
            else if(mg_startswith(message->topic, SPECIAL_TOPICS[1]))
            {
                /*----------------------------------------------------------------------------------------------------*/
                /* GET_PROPERTIES                                                                                     */
                /*----------------------------------------------------------------------------------------------------*/

                for(indi_dict_t **vector_ptr = node->vector_list; *vector_ptr != NULL; vector_ptr++)
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

                /**/ if(strnstr(message->data.ptr, "Never", message->data.len) != NULL) {
                    node->blob = INDI_BLOB_NEVER;
                }
                else if(strnstr(message->data.ptr, "Also", message->data.len) != NULL) {
                    node->blob = INDI_BLOB_ALSO;
                }
                else if(strnstr(message->data.ptr, "Only", message->data.len) != NULL) {
                    node->blob = INDI_BLOB_ONLY;
                }

                /*----------------------------------------------------------------------------------------------------*/

                MG_INFO((">> Blog behavior: %s", indi_blob_to_str(node->blob)));

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
                        update_props(connection->id, node->vector_list, (indi_dict_t *) object);
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
                    indi_object_t *object = indi_xmldoc_to_object(xmldoc, node->validate_xml);

                    if(object != NULL)
                    {
                        if(object->type == INDI_TYPE_DICT)
                        {
                            update_props(connection->id, node->vector_list, (indi_dict_t *) object);
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
    struct indi_node_s *node = (struct indi_node_s *) arg;

    if(node->connection == NULL)
    {
        node->connection = mg_mqtt_connect(&node->mgr, node->url, &node->opts, mqtt_fn, node);
    }
}

/*--------------------------------------------------------------------------------------------------------------------*/

indi_node_t *indi_node_init(
    STR_t url,
    __NULLABLE__ STR_t username,
    __NULLABLE__ STR_t password,
    /**/
    STR_t node_id,
    indi_dict_t *vector_list[],
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

    for(indi_dict_t **vector_ptr = vector_list; *vector_ptr != NULL; vector_ptr++)
    {
        indi_dict_set(*vector_ptr, "@client", indi_string_from(node_id));

        (*vector_ptr)->base.out_callback = out_callback;

        (*vector_ptr)->base.node = node;
    }

    /*----------------------------------------------------------------------------------------------------------------*/
    /* SET NODE OPTIONS                                                                                               */
    /*----------------------------------------------------------------------------------------------------------------*/

    node->url = url;

    node->opts.user = mg_str(username);
    node->opts.pass = mg_str(password);

    node->opts.clean = true;
    node->opts.version = 0x04;

    node->opts.client_id = mg_str(node_id);

    /*----------------------------------------------------------------------------------------------------------------*/

    node->enable_xml = enable_xml;

    node->validate_xml = validate_xml;

    node->blob = INDI_BLOB_NEVER;

    node->vector_list = vector_list;

    /*----------------------------------------------------------------------------------------------------------------*/
    /* INITIALIZE MQTT CLIENT                                                                                         */
    /*----------------------------------------------------------------------------------------------------------------*/

    mg_mgr_init(&node->mgr);

    /*----------------------------------------------------------------------------------------------------------------*/

    mg_timer_add(&node->mgr, retry_ms, MG_TIMER_REPEAT | MG_TIMER_RUN_NOW, timer_fn, node);

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
        for(indi_dict_t **vector_ptr = node->vector_list; *vector_ptr != NULL; vector_ptr++)
        {
            indi_dict_free(*vector_ptr);
        }
    }

    indi_memory_free(node);

    /*----------------------------------------------------------------------------------------------------------------*/
}

/*--------------------------------------------------------------------------------------------------------------------*/
