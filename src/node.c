/*--------------------------------------------------------------------------------------------------------------------*/

#include "mongoose/mongoose.h"

#include "nyx_node_internal.h"

/*--------------------------------------------------------------------------------------------------------------------*/
/* HELPERS                                                                                                            */
/*--------------------------------------------------------------------------------------------------------------------*/

static bool mg_startswith(struct mg_str topic, struct mg_str prefix)
{
    return topic.len >= prefix.len && memcmp(topic.buf, prefix.buf, prefix.len) == 0;
}

/*--------------------------------------------------------------------------------------------------------------------*/

#pragma clang diagnostic push
#pragma ide diagnostic ignored "ConstantParameter"
static void mqtt_pub(struct mg_connection *connection, struct mg_str topic, struct mg_str message, int qos, bool retain)
#pragma clang diagnostic pop
{
    if(connection != NULL)
    {
        struct mg_mqtt_opts opts;

        memset(&opts, 0x00, sizeof(struct mg_mqtt_opts));

        opts.topic = topic;
        opts.message = message;
        opts.qos = qos;
        opts.retain = retain;

        mg_mqtt_pub(connection, &opts);
    }
}

/*--------------------------------------------------------------------------------------------------------------------*/

#pragma clang diagnostic push
#pragma ide diagnostic ignored "ConstantParameter"
static void mqtt_sub(struct mg_connection *connection, struct mg_str topic, int qos)
#pragma clang diagnostic pop
{
    if(connection != NULL)
    {
        struct mg_mqtt_opts opts;

        memset(&opts, 0x00, sizeof(struct mg_mqtt_opts));

        opts.topic = topic;
        ////.message = message;
        opts.qos = qos;
        ////.retain = retain;

        mg_mqtt_sub(connection, &opts);
    }
}

/*--------------------------------------------------------------------------------------------------------------------*/
/* SERVER                                                                                                             */
/*--------------------------------------------------------------------------------------------------------------------*/

struct nyx_node_s
{
    STR_t tcp_url;
    STR_t mqtt_url;

    struct mg_mgr mgr;
    struct mg_mqtt_opts mqtt_opts;
    struct mg_connection *tcp_connection;
    struct mg_connection *mqtt_connection;

    /**/

    nyx_dict_t **def_vectors;

    /**/

    bool enable_xml;
    bool validate_xml;

    /**/

    int last_ping_ms;
};

/*--------------------------------------------------------------------------------------------------------------------*/

#define MG_C_STR(a) {(char *) (a), sizeof(a) - 1}

static struct mg_str SPECIAL_TOPICS[] = {
    MG_C_STR("nyx/cmd/get_clients"),
    MG_C_STR("nyx/cmd/json"),
    MG_C_STR("nyx/cmd/xml"),
};

/*--------------------------------------------------------------------------------------------------------------------*/

static void tcp_sub(struct nyx_node_s *node, STR_t message)
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

static void sub_object(struct nyx_node_s *node, nyx_object_t *object)
{
    /*----------------------------------------------------------------------------------------------------------------*/

    if(node->enable_xml)
    {
        nyx_xmldoc_t *xmldoc = nyx_object_to_xmldoc(object, node->validate_xml);

        if(xmldoc != NULL)
        {
            /*--------------------------------------------------------------------------------------------------------*/

            str_t xml = nyx_xmldoc_to_string(xmldoc);
            mqtt_pub(node->mqtt_connection, mg_str("nyx/xml"), mg_str(xml), 1, false);
            tcp_sub(node, xml);
            nyx_memory_free(xml);

            /*--------------------------------------------------------------------------------------------------------*/

            nyx_xmldoc_free(xmldoc);

            /*--------------------------------------------------------------------------------------------------------*/
        }
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    str_t json = nyx_object_to_string(object);
    mqtt_pub(node->mqtt_connection, mg_str("nyx/json"), mg_str(json), 1, false);
    ////_sub(node, json);
    nyx_memory_free(json);

    /*----------------------------------------------------------------------------------------------------------------*/
}

/*--------------------------------------------------------------------------------------------------------------------*/

static void out_callback(nyx_object_t *object, __UNUSED__ bool modified)
{
    nyx_dict_t *def_vector = (nyx_dict_t *) object;

    if((def_vector->base.flags & NYX_FLAGS_BOTH_DISABLED) == 0)
    {
        STR_t tagname = nyx_dict_get_string(def_vector, "<>");

        if(tagname != NULL)
        {
            /*--------------------------------------------------------------------------------------------------------*/

            nyx_dict_t *set_vector;

            /**/ if(strcmp("defNumberVector", tagname) == 0) {
                set_vector = nyx_number_set_vector_new(def_vector);
            }
            else if(strcmp("defTextVector", tagname) == 0) {
                set_vector = nyx_text_set_vector_new(def_vector);
            }
            else if(strcmp("defLightVector", tagname) == 0) {
                set_vector = nyx_light_set_vector_new(def_vector);
            }
            else if(strcmp("defSwitchVector", tagname) == 0) {
                set_vector = nyx_switch_set_vector_new(def_vector);
            }
            else if(strcmp("defBLOBVector", tagname) == 0) {
                set_vector = nyx_blob_set_vector_new(def_vector);
            }
            else {
                return;
            }

            /*--------------------------------------------------------------------------------------------------------*/

            sub_object(object->node, (nyx_object_t *) set_vector);

            /*--------------------------------------------------------------------------------------------------------*/

            nyx_dict_free(set_vector);

            /*--------------------------------------------------------------------------------------------------------*/
        }
    }
}

/*--------------------------------------------------------------------------------------------------------------------*/

static void get_properties(nyx_node_t *node, nyx_dict_t *dict)
{
    /*----------------------------------------------------------------------------------------------------------------*/

    STR_t device1 = nyx_dict_get_string(dict, "@device");
    STR_t name1 = nyx_dict_get_string(dict, "@name");

    /*----------------------------------------------------------------------------------------------------------------*/

    for(nyx_dict_t **def_vector_ptr = node->def_vectors; *def_vector_ptr != NULL; def_vector_ptr++)
    {
        nyx_dict_t *def_vector = *def_vector_ptr;

        if((def_vector->base.flags & NYX_FLAGS_BOTH_DISABLED) == 0)
        {
            /*--------------------------------------------------------------------------------------------------------*/

            STR_t device2 = nyx_dict_get_string(def_vector, "@device");
            STR_t name2 = nyx_dict_get_string(def_vector, "@name");

            /*--------------------------------------------------------------------------------------------------------*/

            if(device2 != NULL && name2 != NULL)
            {
                /*----------------------------------------------------------------------------------------------------*/

                if(device1 != NULL)
                {
                    if(strcmp(device1, device2) != 0)
                    {
                        continue;
                    }

                    if(name1 != NULL)
                    {
                        if(strcmp(name1, name2) != 0)
                        {
                            continue;
                        }
                    }
                }

                /*----------------------------------------------------------------------------------------------------*/

                sub_object(node, (nyx_object_t *) def_vector);

                /*----------------------------------------------------------------------------------------------------*/
            }

            /*--------------------------------------------------------------------------------------------------------*/
        }
    }

    /*----------------------------------------------------------------------------------------------------------------*/
}

/*--------------------------------------------------------------------------------------------------------------------*/

static void enable_blob(nyx_node_t *node, nyx_dict_t *dict)
{
    /*----------------------------------------------------------------------------------------------------------------*/

    STR_t device1 = nyx_dict_get_string(dict, "@device");
    STR_t name1 = nyx_dict_get_string(dict, "@name");
    STR_t value1 = nyx_dict_get_string(dict, "$");

    /*----------------------------------------------------------------------------------------------------------------*/

    if(device1 != NULL && value1 != NULL)
    {
        nyx_blob_t blob = nyx_str_to_blob(value1);

        /*------------------------------------------------------------------------------------------------------------*/

        for(nyx_dict_t **def_vector_ptr = node->def_vectors; *def_vector_ptr != NULL; def_vector_ptr++)
        {
            nyx_dict_t *def_vector = *def_vector_ptr;

            /*--------------------------------------------------------------------------------------------------------*/

            STR_t device2 = nyx_dict_get_string(def_vector, "@device");
            STR_t name2 = nyx_dict_get_string(def_vector, "@name");
            STR_t tagname2 = nyx_dict_get_string(def_vector, "<>");

            /*--------------------------------------------------------------------------------------------------------*/

            if(device2 != NULL && tagname2 != NULL)
            {
                /*----------------------------------------------------------------------------------------------------*/

                if((strcmp(device1, device2) != 0)
                   ||
                   (name1 != NULL && name2 != NULL && strcmp(name1, name2) != 0)
                ) {
                    continue;
                }

                /*----------------------------------------------------------------------------------------------------*/

                switch(blob)
                {
                    /*------------------------------------------------------------------------------------------------*/

                    case NYX_BLOB_ALSO:
                        def_vector->base.flags &= ~NYX_FLAGS_BLOB_DISABLED;
                        break;

                    /*------------------------------------------------------------------------------------------------*/

                    case NYX_BLOB_NEVER:
                        if(strcmp(tagname2, "defBLOBVector") == 0) {
                            def_vector->base.flags |= NYX_FLAGS_BLOB_DISABLED;
                        }
                        else {
                            def_vector->base.flags &= ~NYX_FLAGS_BLOB_DISABLED;
                        }
                        break;

                    /*------------------------------------------------------------------------------------------------*/

                    case NYX_BLOB_ONLY:
                        if(strcmp(tagname2, "defBLOBVector") == 0) {
                            def_vector->base.flags &= ~NYX_FLAGS_BLOB_DISABLED;
                        }
                        else {
                            def_vector->base.flags |= NYX_FLAGS_BLOB_DISABLED;
                        }
                        break;

                    /*------------------------------------------------------------------------------------------------*/
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

static void set_properties(nyx_node_t *node, nyx_dict_t *dict)
{
    /*----------------------------------------------------------------------------------------------------------------*/

    nyx_object_t *device1_string = nyx_dict_get(dict, "@device");
    nyx_object_t *name1_string = nyx_dict_get(dict, "@name");
    nyx_object_t *children1_list = nyx_dict_get(dict, "children");

    /*----------------------------------------------------------------------------------------------------------------*/

    if(device1_string != NULL && device1_string->type == NYX_TYPE_STRING
       &&
       name1_string != NULL && name1_string->type == NYX_TYPE_STRING
       &&
       children1_list != NULL && children1_list->type == NYX_TYPE_LIST
    ) {
        STR_t device1 = nyx_string_get((nyx_string_t *) device1_string);
        STR_t name1 = nyx_string_get((nyx_string_t *) name1_string);

        /*------------------------------------------------------------------------------------------------------------*/

        for(nyx_dict_t **def_vector_ptr = node->def_vectors; *def_vector_ptr != NULL; def_vector_ptr++)
        {
            nyx_dict_t *def_vector = *def_vector_ptr;

            /*--------------------------------------------------------------------------------------------------------*/

            nyx_object_t *device2_string = nyx_dict_get(def_vector, "@device");
            nyx_object_t *name2_string = nyx_dict_get(def_vector, "@name");
            nyx_object_t *children2_list = nyx_dict_get(def_vector, "children");

            /*--------------------------------------------------------------------------------------------------------*/

            if(device2_string != NULL && device2_string->type == NYX_TYPE_STRING
               &&
               name2_string != NULL && name2_string->type == NYX_TYPE_STRING
               &&
               children2_list != NULL && children1_list->type == NYX_TYPE_LIST
            ) {
                /*----------------------------------------------------------------------------------------------------*/

                STR_t device2 = nyx_string_get((nyx_string_t *) device2_string);
                STR_t name2 = nyx_string_get((nyx_string_t *) name2_string);

                /*----------------------------------------------------------------------------------------------------*/

                if(strcmp(device1, device2) == 0
                   &&
                   strcmp(name1, name2) == 0
                ) {
                    int idx1;
                    int idx2;

                    nyx_object_t *object1;
                    nyx_object_t *object2;

                    bool vector_modified = false;

                    /*------------------------------------------------------------------------------------------------*/

                    STR_t rule = nyx_dict_get_string(def_vector, "@rule");

                    bool is_one_of_many = rule != NULL && strcmp(rule, "OneOfMany") == 0;

                    /*------------------------------------------------------------------------------------------------*/

                    for(nyx_list_iter_t iter1 = NYX_LIST_ITER(children1_list); nyx_list_iterate(&iter1, &idx1, &object1);)
                    {
                        if(object1->type == NYX_TYPE_DICT)
                        {
                            nyx_object_t *prop1_string = nyx_dict_get((nyx_dict_t *) object1, "@name");

                            if(prop1_string != NULL && prop1_string->type == NYX_TYPE_STRING)
                            {
                                STR_t prop1 = nyx_string_get((nyx_string_t *) prop1_string);

                                /*------------------------------------------------------------------------------------*/

                                for(nyx_list_iter_t iter2 = NYX_LIST_ITER(children2_list); nyx_list_iterate(&iter2, &idx2, &object2);)
                                {
                                    if(object2->type == NYX_TYPE_DICT)
                                    {
                                        nyx_object_t *prop2_string = nyx_dict_get((nyx_dict_t *) object2, "@name");

                                        if(prop2_string != NULL && prop2_string->type == NYX_TYPE_STRING)
                                        {
                                            STR_t prop2 = nyx_string_get((nyx_string_t *) prop2_string);

                                            /*------------------------------------------------------------------------*/

                                            bool is_current = strcmp(prop1, prop2) == 0;

                                            if(is_current || is_one_of_many)
                                            {
                                                /*--------------------------------------------------------------------*/

                                                bool modified;

                                                if(is_current) {
                                                    modified = internal_copy((nyx_dict_t *) object2,(nyx_dict_t *) object1, "$", false);
                                                }
                                                else {
                                                    modified = nyx_dict_set2((nyx_dict_t *) object2,"$",nyx_string_static_from("Off"),false);
                                                }

                                                /*--------------------------------------------------------------------*/

                                                vector_modified = vector_modified || modified;

                                                /*--------------------------------------------------------------------*/

                                                str_t str = nyx_object_to_string(object2);
                                                MG_INFO(("Updating (modified: %s) `%s::%s` with %s", modified ? "true" : "false", device1, name1, str));
                                                nyx_memory_free(str);

                                                /*--------------------------------------------------------------------*/

                                                if(object2->in_callback != NULL) object2->in_callback(object2, modified);

                                                /*--------------------------------------------------------------------*/
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

                    if(def_vector->base.in_callback != NULL) def_vector->base.in_callback(&def_vector->base, vector_modified);

                    nyx_object_notify(&def_vector->base, vector_modified);

                    break; /* property found */

                    /*------------------------------------------------------------------------------------------------*/
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

static void process_message(nyx_node_t *node, nyx_object_t *object)
{
    if(object->type == NYX_TYPE_DICT)
    {
        STR_t tagname = nyx_dict_get_string((nyx_dict_t *) object, "<>");

        if(tagname != NULL)
        {
            /**/ if(strcmp(tagname, "getProperties") == 0) {
                get_properties(node, (nyx_dict_t *) object);
            }
            else if(strcmp(tagname, "enableBLOB") == 0) {
                enable_blob(node, (nyx_dict_t *) object);
            }
            else if(strcmp(tagname, "newNumberVector") == 0
                    ||
                    strcmp(tagname, "newTextVector") == 0
                    ||
                    strcmp(tagname, "newLightVector") == 0
                    ||
                    strcmp(tagname, "newSwitchVector") == 0
                    ||
                    strcmp(tagname, "newBLOBVector") == 0
            ) {
                set_properties(node, (nyx_dict_t *) object);
            }
        }
    }
}

/*--------------------------------------------------------------------------------------------------------------------*/

static void tcp_handler(struct mg_connection *connection, int ev, void *ev_data)
{
    nyx_node_t *node = (nyx_node_t *) connection->fn_data;

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

        nyx_stream_t stream = NYX_STREAM();

        struct mg_iobuf *iobuf = &connection->recv;

        if(nyx_stream_detect_opening_tag(&stream, iobuf->len, iobuf->buf))
        {
            if(nyx_stream_detect_closing_tag(&stream, iobuf->len, iobuf->buf))
            {
/*
                for(long i = 0; i < iobuf->len; i++)
                {
                    fprintf(stdout, "%c", iobuf->buf[i]);
                    fflush(stdout);
                }

                fprintf(stdout, "\n\n");
                fflush(stdout);
*/
                /*----------------------------------------------------------------------------------------------------*/

                nyx_xmldoc_t *xmldoc = nyx_xmldoc_parse_buff(stream.s_ptr, stream.len);

                if(xmldoc != NULL)
                {
                    nyx_object_t *object = nyx_xmldoc_to_object(xmldoc, node->validate_xml);

                    if(object != NULL)
                    {
                        process_message(node, object);

                        nyx_object_free(object);
                    }

                    nyx_xmldoc_free(xmldoc);
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
    nyx_node_t *node = (nyx_node_t *) connection->fn_data;

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

        ((nyx_node_t *) connection->fn_data)->mqtt_connection = NULL;
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
            str_t topic = nyx_memory_alloc(SPECIAL_TOPICS[i].len + node->mqtt_opts.client_id.len + 2);

            if(sprintf(topic, "%s/%s", SPECIAL_TOPICS[i].buf, node->mqtt_opts.client_id.buf) > 0)
            {
                MG_INFO(("%lu Subscribing to `%s` and `%s` topics",
                    connection->id,
                    SPECIAL_TOPICS[i].buf,
                    /*-----*/ topic /*-----*/
                ));

                mqtt_sub(connection, SPECIAL_TOPICS[i], 1);

                mqtt_sub(connection, mg_str(topic), 1);
            }

            nyx_memory_free(topic);
        }

        /*------------------------------------------------------------------------------------------------------------*/
    }
    else if(ev == MG_EV_MQTT_MSG)
    {
        /*------------------------------------------------------------------------------------------------------------*/
        /* MG_EV_MQTT_MSG                                                                                             */
        /*------------------------------------------------------------------------------------------------------------*/

        struct mg_mqtt_message *message = (struct mg_mqtt_message *) ev_data;

        if(message->topic.len > 0 && message->topic.buf != NULL
           &&
           message->data.len > 0 && message->data.buf != NULL
        ) {
            /**/ if(mg_startswith(message->topic, SPECIAL_TOPICS[0]))
            {
                /*----------------------------------------------------------------------------------------------------*/
                /* GET_CLIENTS                                                                                        */
                /*----------------------------------------------------------------------------------------------------*/

                mqtt_pub(connection, mg_str("nyx/clients"), node->mqtt_opts.client_id, 1, false);

                /*----------------------------------------------------------------------------------------------------*/
            }
            else if(mg_startswith(message->topic, SPECIAL_TOPICS[1]))
            {
                /*----------------------------------------------------------------------------------------------------*/
                /* JSON NEW XXX VECTOR                                                                                */
                /*----------------------------------------------------------------------------------------------------*/

                nyx_object_t *object = nyx_object_parse(message->data.buf);

                if(object != NULL)
                {
                    process_message(node, object);

                    nyx_object_free(object);
                }

                /*----------------------------------------------------------------------------------------------------*/
            }
            else if(mg_startswith(message->topic, SPECIAL_TOPICS[2]))
            {
                /*----------------------------------------------------------------------------------------------------*/
                /* XML NEW XXX VECTOR                                                                                 */
                /*----------------------------------------------------------------------------------------------------*/

                nyx_xmldoc_t *xmldoc = nyx_xmldoc_parse_buff(message->data.buf, message->data.len);

                if(xmldoc != NULL)
                {
                    nyx_object_t *object = nyx_xmldoc_to_object(xmldoc, node->validate_xml);

                    if(object != NULL)
                    {
                        process_message(node, object);

                        nyx_object_free(object);
                    }

                    nyx_xmldoc_free(xmldoc);
                }

                /*----------------------------------------------------------------------------------------------------*/
            }
        }

        /*------------------------------------------------------------------------------------------------------------*/
    }
}

/*--------------------------------------------------------------------------------------------------------------------*/

static void timer_handler(void *arg)
{
    nyx_node_t *node = (nyx_node_t *) arg;

    if(node->mqtt_connection == NULL)
    {
        node->mqtt_connection = mg_mqtt_connect(
            &node->mgr,
            node->mqtt_url,
            &node->mqtt_opts,
            mqtt_handler,
            node
        );
    }
}

/*--------------------------------------------------------------------------------------------------------------------*/

nyx_node_t *nyx_node_initialize(
    __NULLABLE__ STR_t tcp_url,
    __NULLABLE__ STR_t mqtt_url,
    __NULLABLE__ STR_t username,
    __NULLABLE__ STR_t password,
    /**/
    STR_t node_id,
    nyx_dict_t *def_vectors[],
    /**/
    int retry_ms,
    bool enable_xml,
    bool validate_xml
) {
    /*----------------------------------------------------------------------------------------------------------------*/

    nyx_node_t *node = nyx_memory_alloc(sizeof(nyx_node_t));

    memset(node, 0x00, sizeof(nyx_node_t));

    /*----------------------------------------------------------------------------------------------------------------*/
    /* PATH VECTORS                                                                                                   */
    /*----------------------------------------------------------------------------------------------------------------*/

    for(nyx_dict_t **def_vector_ptr = def_vectors; *def_vector_ptr != NULL; def_vector_ptr++)
    {
        nyx_dict_t *vector = *def_vector_ptr;

        /*------------------------------------------------------------------------------------------------------------*/

        vector->base.out_callback = ((((NULL))));
        nyx_dict_set(vector, "@client", nyx_string_from(node_id));
        vector->base.out_callback = out_callback;

        /*------------------------------------------------------------------------------------------------------------*/

        int idx;
        nyx_object_t *object;

        for(nyx_list_iter_t iter = NYX_LIST_ITER((nyx_list_t *) nyx_dict_get(vector, "children")); nyx_list_iterate(&iter, &idx, &object);)
        {
            object->/**/node/**/ = node;
        }

        /*------------------------------------------------------------------------------------------------------------*/

        vector->base.node = node;

        /*------------------------------------------------------------------------------------------------------------*/
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

    if(mqtt_url != NULL)
    {
        mg_timer_add(&node->mgr, retry_ms, MG_TIMER_REPEAT | MG_TIMER_RUN_NOW, timer_handler, node);
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    return node;
}

/*--------------------------------------------------------------------------------------------------------------------*/

void nyx_node_pool(nyx_node_t *node, int timeout_ms)
{
    /*----------------------------------------------------------------------------------------------------------------*/

    mg_mgr_poll(&node->mgr, timeout_ms);

    /*----------------------------------------------------------------------------------------------------------------*/

    node->last_ping_ms += timeout_ms;

    if(node->last_ping_ms >= INI_PING_MS)
    {
        node->last_ping_ms = 0x00000000000000;

        mqtt_pub(node->mqtt_connection, mg_str("nyx/ping/node"), node->mqtt_opts.client_id, 1, false);
    }

    /*----------------------------------------------------------------------------------------------------------------*/
}

/*--------------------------------------------------------------------------------------------------------------------*/

void nyx_node_free(nyx_node_t *node, bool free_vectors)
{
    /*----------------------------------------------------------------------------------------------------------------*/

    mg_mgr_free(&node->mgr);

    /*----------------------------------------------------------------------------------------------------------------*/

    if(free_vectors)
    {
        for(nyx_dict_t **def_vector_ptr = node->def_vectors; *def_vector_ptr != NULL; def_vector_ptr++)
        {
            nyx_dict_free(*def_vector_ptr);
        }
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    nyx_memory_free(node);

    /*----------------------------------------------------------------------------------------------------------------*/
}

/*--------------------------------------------------------------------------------------------------------------------*/

void nyx_node_enable(nyx_node_t *node, STR_t device, __NULLABLE__ STR_t name)
{
    internal_mask(node->def_vectors, device, name, NYX_FLAGS_XXXX_DISABLED, true);
}

/*--------------------------------------------------------------------------------------------------------------------*/

void nyx_node_disable(nyx_node_t *node, STR_t device, __NULLABLE__ STR_t name)
{
    internal_mask(node->def_vectors, device, name, NYX_FLAGS_XXXX_DISABLED, false);
}

/*--------------------------------------------------------------------------------------------------------------------*/

void nyx_node_send_message(nyx_node_t *node, STR_t device, STR_t message)
{
    if(node != NULL)
    {
        nyx_dict_t *dict = nyx_message_new(device, message);

        sub_object(node, (nyx_object_t *) dict);

        nyx_dict_free(dict);
    }
}

/*--------------------------------------------------------------------------------------------------------------------*/

void nyx_node_send_del_property(nyx_node_t *node, STR_t device, __NULLABLE__ STR_t name, __NULLABLE__ STR_t message)
{
    if(node != NULL)
    {
        nyx_dict_t *dict = nyx_del_property_new(device, name, message);

        sub_object(node, (nyx_object_t *) dict);

        nyx_dict_free(dict);
    }
}

/*--------------------------------------------------------------------------------------------------------------------*/
