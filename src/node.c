/* NyxNode
 * Author: Jérôme ODIER <jerome.odier@lpsc.in2p3.fr>
 * SPDX-License-Identifier: GPL-2.0-only (Mongoose backend) or GPL-3.0+
 */

/*--------------------------------------------------------------------------------------------------------------------*/

#include <stdio.h>
#include <string.h>

#include "nyx_node_internal.h"

/*--------------------------------------------------------------------------------------------------------------------*/
/* HELPERS                                                                                                            */
/*--------------------------------------------------------------------------------------------------------------------*/

static bool nyx_startswith(nyx_str_t topic, nyx_str_t prefix)
{
    return topic.len >= prefix.len && memcmp(topic.buf, prefix.buf, prefix.len) == 0;
}

/*--------------------------------------------------------------------------------------------------------------------*/
/* NODE                                                                                                               */
/*--------------------------------------------------------------------------------------------------------------------*/

#define NYX_C_STR(a) {(str_t) (a), sizeof(a) - 1}

static nyx_str_t SPECIAL_TOPICS[] = {
    NYX_C_STR("nyx/cmd/trigger_ping"),
    NYX_C_STR("nyx/cmd/set_master_client"),
    NYX_C_STR("nyx/cmd/json"),
    NYX_C_STR("nyx/cmd/xml"),
};

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
            nyx_mqtt_pub(node, nyx_str_s("nyx/xml"), nyx_str_s(xml));
            nyx_tcp_pub(node, nyx_str_s(xml));
            nyx_memory_free(xml);

            /*--------------------------------------------------------------------------------------------------------*/

            nyx_xmldoc_free(xmldoc);

            /*--------------------------------------------------------------------------------------------------------*/
        }
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    str_t json = nyx_object_to_string(object);
    nyx_mqtt_pub(node, nyx_str_s("nyx/json"), nyx_str_s(json));
    ///_tcp_pub(node, nyx_str_s(json));
    nyx_memory_free(json);

    /*----------------------------------------------------------------------------------------------------------------*/
}

/*--------------------------------------------------------------------------------------------------------------------*/

static void out_callback(nyx_object_t *object, __UNUSED__ bool modified)
{
    nyx_dict_t *def_vector = (nyx_dict_t *) object;

    if((def_vector->base.flags & NYX_FLAGS_BOTH_DISABLED) == 0)
    {
        STR_t tag = nyx_dict_get_string(def_vector, "<>");

        if(tag != NULL)
        {
            /*--------------------------------------------------------------------------------------------------------*/

            nyx_dict_t *set_vector;

            /**/ if(strcmp("defNumberVector", tag) == 0) {
                set_vector = nyx_number_set_vector_new(def_vector);
            }
            else if(strcmp("defTextVector", tag) == 0) {
                set_vector = nyx_text_set_vector_new(def_vector);
            }
            else if(strcmp("defLightVector", tag) == 0) {
                set_vector = nyx_light_set_vector_new(def_vector);
            }
            else if(strcmp("defSwitchVector", tag) == 0) {
                set_vector = nyx_switch_set_vector_new(def_vector);
            }
            else if(strcmp("defBLOBVector", tag) == 0) {
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
    /* GET PROPERTIES                                                                                                 */
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
            STR_t tag2 = nyx_dict_get_string(def_vector, "<>");

            /*--------------------------------------------------------------------------------------------------------*/

            if(device2 == NULL || strcmp(device1, device2) != 0)
            {
                continue;
            }

            if(name1 != NULL)
            {
                if(name2 == NULL || strcmp(name1, name2) != 0)
                {
                    continue;
                }
            }

            /*--------------------------------------------------------------------------------------------------------*/

            switch(blob)
            {
                /*----------------------------------------------------------------------------------------------------*/

                case NYX_BLOB_ALSO:
                    def_vector->base.flags &= ~NYX_FLAGS_BLOB_DISABLED;
                    break;

                /*----------------------------------------------------------------------------------------------------*/

                case NYX_BLOB_NEVER:
                    if(tag2 != NULL && strcmp(tag2, "defBLOBVector") == 0) {
                        def_vector->base.flags |= NYX_FLAGS_BLOB_DISABLED;
                    }
                    else {
                        def_vector->base.flags &= ~NYX_FLAGS_BLOB_DISABLED;
                    }
                    break;

                /*----------------------------------------------------------------------------------------------------*/

                case NYX_BLOB_ONLY:
                    if(tag2 != NULL && strcmp(tag2, "defBLOBVector") == 0) {
                        def_vector->base.flags &= ~NYX_FLAGS_BLOB_DISABLED;
                    }
                    else {
                        def_vector->base.flags |= NYX_FLAGS_BLOB_DISABLED;
                    }
                    break;

                /*----------------------------------------------------------------------------------------------------*/
            }

            /*--------------------------------------------------------------------------------------------------------*/
        }

        /*------------------------------------------------------------------------------------------------------------*/
    }

    /*----------------------------------------------------------------------------------------------------------------*/
}

/*--------------------------------------------------------------------------------------------------------------------*/

static bool is_allowed(nyx_node_t *node, nyx_dict_t *dict)
{
    /*----------------------------------------------------------------------------------------------------------------*/

    STR_t client1 = node->master_client_message.buf;

    if((client1 != NULL) && (strcmp("@ALL", client1) == 0))
    {
        return true;
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    STR_t client2 = nyx_dict_get_string(dict, "@client");

    if((client1 != NULL) && (client2 != NULL) && (strcmp(client1, client2) == 0))
    {
        return true;
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    return false;
}

/*--------------------------------------------------------------------------------------------------------------------*/

static void set_properties(nyx_node_t *node, nyx_dict_t *dict)
{
    if(!is_allowed(node, dict))
    {
        return;
    }

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
                                                NYX_LOG_DEBUG("Updating (modified: %s) `%s::%s` with %s", modified ? "true" : "false", device1, name1, str);
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
        STR_t tag = nyx_dict_get_string((nyx_dict_t *) object, "<>");

        if(tag != NULL)
        {
            /**/ if(strcmp(tag, "getProperties") == 0) {
                get_properties(node, (nyx_dict_t *) object);
            }
            else if(strcmp(tag, "enableBLOB") == 0) {
                enable_blob(node, (nyx_dict_t *) object);
            }
            else if(strcmp(tag, "newNumberVector") == 0
                    ||
                    strcmp(tag, "newTextVector") == 0
                    ||
                    strcmp(tag, "newLightVector") == 0
                    ||
                    strcmp(tag, "newSwitchVector") == 0
                    ||
                    strcmp(tag, "newBLOBVector") == 0
            ) {
                set_properties(node, (nyx_dict_t *) object);
            }
        }
    }
}

/*--------------------------------------------------------------------------------------------------------------------*/

static size_t tcp_handler(nyx_node_t *node, int event_type, size_t size, BUFF_t buff)
{
    if(event_type == NYX_EVENT_MSG)
    {
        nyx_stream_t stream = NYX_STREAM();

        if(nyx_stream_detect_opening_tag(&stream, size, buff))
        {
            if(nyx_stream_detect_closing_tag(&stream, size, buff))
            {
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

                return stream.pos + stream.len;

                /*----------------------------------------------------------------------------------------------------*/
            }
        }
    }

    return 0;
}

/*--------------------------------------------------------------------------------------------------------------------*/

static void mqtt_handler(nyx_node_t *node, int event_type, nyx_str_t event_topic, nyx_str_t event_message)
{
    if(event_type == NYX_EVENT_OPEN)
    {
        /*------------------------------------------------------------------------------------------------------------*/
        /* MG_EV_MQTT_OPEN                                                                                            */
        /*------------------------------------------------------------------------------------------------------------*/

        for(int i = 0; i < sizeof(SPECIAL_TOPICS) / sizeof(nyx_str_t); i++)
        {
            str_t topic = nyx_memory_alloc(SPECIAL_TOPICS[i].len + node->node_id.len + 2);

            if(sprintf(topic, "%s/%s", SPECIAL_TOPICS[i].buf, node->node_id.buf) > 0)
            {
                NYX_LOG_INFO("Subscribing to `%s` and `%s` topics",
                     SPECIAL_TOPICS[i].buf,
                     /*---*/ topic /*---*/
                );

                nyx_mqtt_sub(node, SPECIAL_TOPICS[i]);

                nyx_mqtt_sub(node, nyx_str_s(topic));
            }

            nyx_memory_free(topic);
        }

        /*------------------------------------------------------------------------------------------------------------*/
    }
    else if(event_type == NYX_EVENT_MSG)
    {
        /*------------------------------------------------------------------------------------------------------------*/
        /* MG_EV_MQTT_MSG                                                                                             */
        /*------------------------------------------------------------------------------------------------------------*/

        if(event_topic.len > 0 && event_topic.buf != NULL)
        {
            /**/ if(nyx_startswith(event_topic, SPECIAL_TOPICS[0]))
            {
                /*----------------------------------------------------------------------------------------------------*/
                /* TRIGGER PING                                                                                       */
                /*----------------------------------------------------------------------------------------------------*/

                nyx_node_ping(node);

                /*----------------------------------------------------------------------------------------------------*/
            }
            else
            {
                if(event_message.len > 0 && event_message.buf != NULL)
                {
                    /**/ if(nyx_startswith(event_topic, SPECIAL_TOPICS[1]))
                    {
                        /*--------------------------------------------------------------------------------------------*/
                        /* SET_MASTER_CLIENT                                                                          */
                        /*--------------------------------------------------------------------------------------------*/

                        nyx_memory_free(node->master_client_message.buf);

                        node->master_client_message.buf = nyx_memory_alloc(event_message.len + 1);

                        strncpy(node->master_client_message.buf, event_message.buf, event_message.len)[node->master_client_message.len = event_message.len] = '\0';

                        /*--------------------------------------------------------------------------------------------*/
                    }
                    else if(nyx_startswith(event_topic, SPECIAL_TOPICS[2]))
                    {
                        /*--------------------------------------------------------------------------------------------*/
                        /* JSON NEW XXX VECTOR                                                                        */
                        /*--------------------------------------------------------------------------------------------*/

                        nyx_object_t *object = nyx_object_parse_buff(event_message.buf, event_message.len);

                        if(object != NULL)
                        {
                            process_message(node, object);

                            nyx_object_free(object);
                        }

                        /*--------------------------------------------------------------------------------------------*/
                    }
                    else if(nyx_startswith(event_topic, SPECIAL_TOPICS[3]))
                    {
                        /*--------------------------------------------------------------------------------------------*/
                        /* XML NEW XXX VECTOR                                                                         */
                        /*--------------------------------------------------------------------------------------------*/

                        nyx_xmldoc_t *xmldoc = nyx_xmldoc_parse_buff(event_message.buf, event_message.len);

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

                        /*--------------------------------------------------------------------------------------------*/
                    }
                }
            }
        }

        /*------------------------------------------------------------------------------------------------------------*/
    }
}

/*--------------------------------------------------------------------------------------------------------------------*/

nyx_node_t *nyx_node_initialize(
    __NULLABLE__ STR_t tcp_url,
    __NULLABLE__ STR_t mqtt_url,
    __NULLABLE__ STR_t mqtt_username,
    __NULLABLE__ STR_t mqtt_password,
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

    node->node_id = nyx_str_s(node_id);

    /*----------------------------------------------------------------------------------------------------------------*/

    str_t master_client_topic = nyx_memory_alloc(strlen("nyx/master_client/") + node->node_id.len + 1);

    if(sprintf(master_client_topic, "nyx/master_client/%s", node->node_id.buf) > 0)
    {
        node->master_client_message = nyx_str_s(nyx_string_dup("@ALL"));

        node->master_client_topic = nyx_str_s(master_client_topic);
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    node->tcp_url = tcp_url;
    node->mqtt_url = mqtt_url;

    node->enable_xml = enable_xml;
    node->validate_xml = validate_xml;

    node->def_vectors = def_vectors;

    node->tcp_handler = tcp_handler;
    node->mqtt_handler = mqtt_handler;

    /*----------------------------------------------------------------------------------------------------------------*/
    /* INITIALIZE STACK                                                                                               */
    /*----------------------------------------------------------------------------------------------------------------*/

    nyx_node_stack_initialize(node, mqtt_username, mqtt_password, retry_ms);

    /*----------------------------------------------------------------------------------------------------------------*/

    return node;
}

/*--------------------------------------------------------------------------------------------------------------------*/

void nyx_node_finalize(nyx_node_t *node, bool free_vectors)
{
    /*----------------------------------------------------------------------------------------------------------------*/
    /* FINALIZE STACK                                                                                                 */
    /*----------------------------------------------------------------------------------------------------------------*/

    nyx_node_stack_finalize(node);

    /*----------------------------------------------------------------------------------------------------------------*/
    /* FREE DEF VECTORS                                                                                               */
    /*----------------------------------------------------------------------------------------------------------------*/

    if(free_vectors)
    {
        for(nyx_dict_t **def_vector_ptr = node->def_vectors; *def_vector_ptr != NULL; def_vector_ptr++)
        {
            nyx_dict_free(*def_vector_ptr);
        }
    }

    /*----------------------------------------------------------------------------------------------------------------*/
    /* FREE NODE                                                                                                      */
    /*----------------------------------------------------------------------------------------------------------------*/

    nyx_memory_free(node->master_client_message.buf);

    nyx_memory_free(node->master_client_topic.buf);

    /*----------------------------------------------------------------------------------------------------------------*/

    nyx_memory_free(node);

    /*----------------------------------------------------------------------------------------------------------------*/
}

/*--------------------------------------------------------------------------------------------------------------------*/

void nyx_node_ping(nyx_node_t *node)
{
    nyx_mqtt_pub(node, nyx_str_s("nyx/ping/node"), node->node_id);

    nyx_mqtt_pub(node, node->master_client_topic, node->master_client_message);
}

/*--------------------------------------------------------------------------------------------------------------------*/

static void device_onoff(nyx_node_t *node, STR_t device, __NULLABLE__ STR_t name, __NULLABLE__ STR_t message, nyx_onoff_t onoff)
{
    /*----------------------------------------------------------------------------------------------------------------*/

    if(device != NULL)
    {
        /*------------------------------------------------------------------------------------------------------------*/

        for(nyx_dict_t **def_vector_ptr = node->def_vectors; *def_vector_ptr != NULL; def_vector_ptr++)
        {
            nyx_dict_t *def_vector = *def_vector_ptr;

            /*--------------------------------------------------------------------------------------------------------*/

            STR_t device2 = nyx_dict_get_string(def_vector, "@device");
            STR_t name2 = nyx_dict_get_string(def_vector, "@name");

            /*--------------------------------------------------------------------------------------------------------*/

            if(device2 == NULL || strcmp(device, device2) != 0)
            {
                continue;
            }

            if(name != NULL)
            {
                if(name2 == NULL || strcmp(name, name2) != 0)
                {
                    continue;
                }
            }

            /*--------------------------------------------------------------------------------------------------------*/

            switch(onoff)
            {
                case NYX_ONOFF_OFF:
                    def_vector->base.flags |= NYX_FLAGS_XXXX_DISABLED;
                    break;

                case NYX_ONOFF_ON:
                    def_vector->base.flags &= ~NYX_FLAGS_XXXX_DISABLED;

                    sub_object(node, (nyx_object_t *) def_vector);
                    break;
            }

            /*--------------------------------------------------------------------------------------------------------*/
        }

        /*------------------------------------------------------------------------------------------------------------*/

        if(onoff == NYX_ONOFF_OFF)
        {
            nyx_dict_t *del_property_new = nyx_del_property_new(device, name, message);

            sub_object(node, (nyx_object_t *) del_property_new);

            nyx_dict_free(del_property_new);
        }

        /*------------------------------------------------------------------------------------------------------------*/
    }
}

/*--------------------------------------------------------------------------------------------------------------------*/

void nyx_node_enable(nyx_node_t *node, STR_t device, __NULLABLE__ STR_t name, __NULLABLE__ STR_t message)
{
    device_onoff(node, device, name, message, NYX_ONOFF_ON);
}

/*--------------------------------------------------------------------------------------------------------------------*/

void nyx_node_disable(nyx_node_t *node, STR_t device, __NULLABLE__ STR_t name, __NULLABLE__ STR_t message)
{
    device_onoff(node, device, name, message, NYX_ONOFF_OFF);
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
