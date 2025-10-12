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

static const nyx_str_t SPECIAL_TOPICS[] = {
    NYX_C_STR("nyx/cmd/trigger_ping"),
    NYX_C_STR("nyx/cmd/set_master_client"),
    NYX_C_STR("nyx/cmd/json"),
    NYX_C_STR("nyx/cmd/xml"),
};

/*--------------------------------------------------------------------------------------------------------------------*/

static void _sub_object(struct nyx_node_s *node, const nyx_object_t *object)
{
    /*----------------------------------------------------------------------------------------------------------------*/

    if(node->enable_xml)
    {
        nyx_xmldoc_t *xmldoc = nyx_object_to_xmldoc(object);

        if(xmldoc != NULL)
        {
            /*--------------------------------------------------------------------------------------------------------*/

            str_t xml = nyx_xmldoc_to_string(xmldoc);
            internal_mqtt_pub(node, nyx_str_s("nyx/xml"), nyx_str_s(xml));
            internal_indi_pub(node, nyx_str_s(xml));
            nyx_memory_free(xml);

            /*--------------------------------------------------------------------------------------------------------*/

            nyx_xmldoc_free(xmldoc);

            /*--------------------------------------------------------------------------------------------------------*/
        }
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    str_t json = nyx_object_to_string(object);
    internal_mqtt_pub(node, nyx_str_s("nyx/json"), nyx_str_s(json));
    ////////_indi_pub(node, nyx_str_s(json));
    nyx_memory_free(json);

    /*----------------------------------------------------------------------------------------------------------------*/
}

/*--------------------------------------------------------------------------------------------------------------------*/

static void _out_callback(nyx_object_t *object)
{
    nyx_dict_t *vector = (nyx_dict_t *) object;

    if((vector->base.flags & NYX_FLAGS_DISABLED) == 0)
    {
        STR_t tag = nyx_dict_get_string(vector, "<>");

        if(tag != NULL)
        {
            /*--------------------------------------------------------------------------------------------------------*/

            nyx_dict_t *set_vector;

            /**/ if(strcmp("defNumberVector", tag) == 0) {
                set_vector = nyx_number_set_vector_new(vector);
            }
            else if(strcmp("defTextVector", tag) == 0) {
                set_vector = nyx_text_set_vector_new(vector);
            }
            else if(strcmp("defLightVector", tag) == 0) {
                set_vector = nyx_light_set_vector_new(vector);
            }
            else if(strcmp("defSwitchVector", tag) == 0) {
                set_vector = nyx_switch_set_vector_new(vector);
            }
            else if(strcmp("defStreamVector", tag) == 0) {
                set_vector = nyx_stream_set_vector_new(vector);
            }
            else if(strcmp("defBLOBVector", tag) == 0) {

                //if((vector->base.flags & NYX_FLAGS_BLOB_MASK) != 0) {

                    set_vector = nyx_blob_set_vector_new(vector);
                //}
                //else {
                //    return;
                //}
            }
            else {
                return;
            }

            /*--------------------------------------------------------------------------------------------------------*/

            STR_t perm = nyx_dict_get_string(vector, "@perm");

            bool is_not_wo = perm == NULL || strcmp(perm, "wo") != 0;

            if(is_not_wo) _sub_object(object->node, (nyx_object_t *) set_vector);

            /*--------------------------------------------------------------------------------------------------------*/

            nyx_dict_free(set_vector);

            /*--------------------------------------------------------------------------------------------------------*/
        }
    }
}

/*--------------------------------------------------------------------------------------------------------------------*/

static void _get_properties_unsafe(nyx_node_t *node, __NULLABLE__ const nyx_dict_t *dict)
{
    /*----------------------------------------------------------------------------------------------------------------*/
    /* GET PROPERTIES                                                                                                 */
    /*----------------------------------------------------------------------------------------------------------------*/

    STR_t device1;
    STR_t name1;

    if(dict != NULL)
    {
        device1 = nyx_dict_get_string(dict, "@device");
        name1 = nyx_dict_get_string(dict, "@name");
    }
    else
    {
        device1 = NULL;
        name1 = NULL;
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    for(nyx_dict_t **vector_ptr = node->vectors; *vector_ptr != NULL; vector_ptr++)
    {
        nyx_dict_t *vector = *vector_ptr;

        if((vector->base.flags & NYX_FLAGS_DISABLED) == 0)
        {
            /*--------------------------------------------------------------------------------------------------------*/

            STR_t device2 = nyx_dict_get_string(vector, "@device");
            STR_t name2 = nyx_dict_get_string(vector, "@name");

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

                _sub_object(node, (nyx_object_t *) vector);

                /*----------------------------------------------------------------------------------------------------*/
            }

            /*--------------------------------------------------------------------------------------------------------*/
        }
    }

    /*----------------------------------------------------------------------------------------------------------------*/
}

/*--------------------------------------------------------------------------------------------------------------------*/

static int _get_client_index(nyx_node_t *node, __NULLABLE__ STR_t client)
{
    if(client == NULL)
    {
        client = "@INDI";
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    uint32_t hash = nyx_hash32(strlen(client), client, NYX_OBJECT_MAGIC);

    /*----------------------------------------------------------------------------------------------------------------*/

    for(size_t i = 0; i < sizeof(node->client_hashes) / sizeof(uint32_t); i++)
    {
        if(node->client_hashes[i] == 0x00
           ||
           node->client_hashes[i] == hash
        ) {
            node->client_hashes[i] = hash;

            return (int) i;
        }
    }

    return -1;

    /*----------------------------------------------------------------------------------------------------------------*/
}

/*--------------------------------------------------------------------------------------------------------------------*/

static void _enable_xxx(nyx_node_t *node, const nyx_dict_t *dict, STR_t tag, int (* str_to_xxx)(STR_t))
{
    /*----------------------------------------------------------------------------------------------------------------*/

    STR_t client = nyx_dict_get_string(dict, "@client");

    int index = _get_client_index(node, client);

    if(index < 0)
    {
        return;
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    STR_t device1 = nyx_dict_get_string(dict, "@device");
    STR_t name1 = nyx_dict_get_string(dict, "@name");
    STR_t value1 = nyx_dict_get_string(dict, "$");

    /*----------------------------------------------------------------------------------------------------------------*/

    if(device1 != NULL && value1 != NULL)
    {
        int value = str_to_xxx(value1);

        /*------------------------------------------------------------------------------------------------------------*/

        for(nyx_dict_t **vector_ptr = node->vectors; *vector_ptr != NULL; vector_ptr++)
        {
            nyx_dict_t *vector = *vector_ptr;

            /*--------------------------------------------------------------------------------------------------------*/

            STR_t device2 = nyx_dict_get_string(vector, "@device");
            STR_t name2 = nyx_dict_get_string(vector, "@name");
            STR_t tag2 = nyx_dict_get_string(vector, "<>");

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

            if(tag2 != NULL && strcmp(tag2, tag) == 0)
            {
                switch(value)
                {
                    /*------------------------------------------------------------------------------------------------*/
                    /* BLOB                                                                                           */
                    /*------------------------------------------------------------------------------------------------*/

                    case NYX_BLOB_STATE_ENABLED:
                        vector->base.flags |= (1LU << (2 + 0 * 31 + index));
                        break;

                    case NYX_BLOB_STATE_DISABLED:
                        vector->base.flags &= ~(1LU << (2 + 0 * 31 + index));
                        break;

                    /*------------------------------------------------------------------------------------------------*/
                    /* STREAM                                                                                         */
                    /*------------------------------------------------------------------------------------------------*/

                    case NYX_STREAM_STATE_ENABLED:
                        vector->base.flags |= (1LU << (2 + 1 * 31 + index));
                        break;

                    case NYX_STREAM_STATE_DISABLED:
                        vector->base.flags &= ~(1LU << (2 + 1 * 31 + index));
                        break;

                    /*------------------------------------------------------------------------------------------------*/
                    /* INTERNAL ERROR                                                                                 */
                    /*------------------------------------------------------------------------------------------------*/

                    default:
                        NYX_LOG_FATAL("Internal error");

                    /*------------------------------------------------------------------------------------------------*/
                }
            }

            /*--------------------------------------------------------------------------------------------------------*/
        }

        /*------------------------------------------------------------------------------------------------------------*/
    }

    /*----------------------------------------------------------------------------------------------------------------*/
}

/*--------------------------------------------------------------------------------------------------------------------*/

__INLINE__ void _enable_blob_unsafe(nyx_node_t *node, const nyx_dict_t *dict)
{
    _enable_xxx(node, dict, "defBLOBVector", (int (*)(STR_t)) nyx_str_to_blob_state);
}

/*--------------------------------------------------------------------------------------------------------------------*/

__INLINE__ void _enable_stream_unsafe(nyx_node_t *node, const nyx_dict_t *dict)
{
    _enable_xxx(node, dict, "defStreamVector", (int (*)(STR_t)) nyx_str_to_stream_state);
}

/*--------------------------------------------------------------------------------------------------------------------*/

static bool _is_allowed(const nyx_node_t *node, const nyx_dict_t *dict)
{
    /*----------------------------------------------------------------------------------------------------------------*/

    STR_t client1 = node->master_client_message.buf;

    if((client1 != NULL) && (strcmp(NYX_ALL, client1) == 0))
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

static nyx_string_t OFF = {
    .base = NYX_OBJECT(NYX_TYPE_STRING),
    .raw_size = 0x003,
    .length = 0x003,
    .value = "Off",
    .dyn = false,
};

/*--------------------------------------------------------------------------------------------------------------------*/

static void _set_properties_unsafe(const nyx_node_t *node, const nyx_dict_t *dict)
{
    if(!_is_allowed(node, dict))
    {
        return;
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    nyx_object_t *tag1_string = nyx_dict_get(dict, "<>");
    nyx_object_t *device1_string = nyx_dict_get(dict, "@device");
    nyx_object_t *name1_string = nyx_dict_get(dict, "@name");
    nyx_object_t *children1_list = nyx_dict_get(dict, "children");

    /*----------------------------------------------------------------------------------------------------------------*/

    if(tag1_string != NULL && device1_string->type == NYX_TYPE_STRING
       &&
       device1_string != NULL && device1_string->type == NYX_TYPE_STRING
       &&
       name1_string != NULL && name1_string->type == NYX_TYPE_STRING
       &&
       children1_list != NULL && children1_list->type == NYX_TYPE_LIST
    ) {
        /*------------------------------------------------------------------------------------------------------------*/

        STR_t tag1 = nyx_string_get((nyx_string_t *) tag1_string);
        STR_t device1 = nyx_string_get((nyx_string_t *) device1_string);
        STR_t name1 = nyx_string_get((nyx_string_t *) name1_string);

        /*------------------------------------------------------------------------------------------------------------*/

        for(nyx_dict_t **vector_ptr = node->vectors; *vector_ptr != NULL; vector_ptr++)
        {
            nyx_dict_t *vector = *vector_ptr;

            /*--------------------------------------------------------------------------------------------------------*/

            nyx_object_t *tag2_string = nyx_dict_get(vector, "<>");
            nyx_object_t *device2_string = nyx_dict_get(vector, "@device");
            nyx_object_t *name2_string = nyx_dict_get(vector, "@name");
            nyx_object_t *children2_list = nyx_dict_get(vector, "children");

            /*--------------------------------------------------------------------------------------------------------*/

            if(tag2_string != NULL && device2_string->type == NYX_TYPE_STRING
               &&
               device2_string != NULL && device2_string->type == NYX_TYPE_STRING
               &&
               name2_string != NULL && name2_string->type == NYX_TYPE_STRING
               &&
               children2_list != NULL && children2_list->type == NYX_TYPE_LIST
            ) {
                /*----------------------------------------------------------------------------------------------------*/

                STR_t tag2 = nyx_string_get((nyx_string_t *) tag2_string);
                STR_t device2 = nyx_string_get((nyx_string_t *) device2_string);
                STR_t name2 = nyx_string_get((nyx_string_t *) name2_string);

                /*----------------------------------------------------------------------------------------------------*/

                if(strlen(tag1) > 3 && strlen(tag2) > 3     // skip "def" and "new" suffixes
                   &&
                   strcmp(tag1 + 3, tag2 + 3) == 0
                   &&
                   strcmp(device1, device2) == 0
                   &&
                   strcmp(name1, name2) == 0
                ) {
                    int idx1;
                    int idx2;

                    nyx_object_t *object1;
                    nyx_object_t *object2;

                    bool vector_modified = false;

                    uint32_t hash = nyx_hash32(strlen(tag2), tag2, 0);

                    /*------------------------------------------------------------------------------------------------*/

                    STR_t rule = nyx_dict_get_string(vector, "@rule");

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

                                                nyx_object_t *old_value = /*--------*/ nyx_dict_get((nyx_dict_t *) object2, "$");
                                                nyx_object_t *new_value = is_current ? nyx_dict_get((nyx_dict_t *) object1, "$")
                                                                                     : (nyx_object_t *) &OFF
                                                ;

                                                /*--------------------------------------------------------------------*/

                                                bool success = false;
                                                bool modified = false;

                                                switch(hash)
                                                {
                                                    /*----------------------------------------------------------------*/

                                                    case 0x56BE29BD:    // defNumberVector
                                                        {
                                                            nyx_object_t *format_string = nyx_dict_get((nyx_dict_t *) object2, "@format");

                                                            if(format_string != NULL && format_string->type == NYX_TYPE_STRING)
                                                            {
                                                                STR_t format = nyx_string_get((nyx_string_t *) format_string);

                                                                nyx_variant_t old_val = internal_string_to_variant(format, (nyx_string_t *) old_value);
                                                                nyx_variant_t new_val = internal_string_to_variant(format, (nyx_string_t *) new_value);

                                                                switch(new_val.type)
                                                                {
                                                                    case NYX_VARIANT_TYPE_INT:
                                                                        if((success = object2->in_callback._int == NULL || object2->in_callback._int(vector, (nyx_dict_t *) object2, new_val.value._int, old_val.value._int))) {
                                                                            modified = nyx_dict_set_alt((nyx_dict_t *) object2, "$", internal_variant_to_string(format, new_val), false);
                                                                        }
                                                                        break;
                                                                    case NYX_VARIANT_TYPE_UINT:
                                                                        if((success = object2->in_callback._uint == NULL || object2->in_callback._uint(vector, (nyx_dict_t *) object2, new_val.value._uint, old_val.value._uint))) {
                                                                            modified = nyx_dict_set_alt((nyx_dict_t *) object2, "$", internal_variant_to_string(format, new_val), false);
                                                                        }
                                                                        break;
                                                                    case NYX_VARIANT_TYPE_LONG:
                                                                        if((success = object2->in_callback._long == NULL || object2->in_callback._long(vector, (nyx_dict_t *) object2, new_val.value._long, old_val.value._long))) {
                                                                            modified = nyx_dict_set_alt((nyx_dict_t *) object2, "$", internal_variant_to_string(format, new_val), false);
                                                                        }
                                                                        break;
                                                                    case NYX_VARIANT_TYPE_ULONG:
                                                                        if((success = object2->in_callback._ulong == NULL || object2->in_callback._ulong(vector, (nyx_dict_t *) object2, new_val.value._ulong, old_val.value._ulong))) {
                                                                            modified = nyx_dict_set_alt((nyx_dict_t *) object2, "$", internal_variant_to_string(format, new_val), false);
                                                                        }
                                                                        break;
                                                                    case NYX_VARIANT_TYPE_DOUBLE:
                                                                        if((success = object2->in_callback._double == NULL || object2->in_callback._double(vector, (nyx_dict_t *) object2, new_val.value._double, old_val.value._double))) {
                                                                            modified = nyx_dict_set_alt((nyx_dict_t *) object2, "$", internal_variant_to_string(format, new_val), false);
                                                                        }
                                                                        break;
                                                                }
                                                            }
                                                        }

                                                        break;

                                                    /*----------------------------------------------------------------*/

                                                    case 0x1FD73301:    // defTextVector
                                                        {
                                                            STR_t old_val = nyx_string_get((nyx_string_t *) old_value);
                                                            STR_t new_val = nyx_string_get((nyx_string_t *) new_value);

                                                            if((success = object2->in_callback._str == NULL || object2->in_callback._str(vector, (nyx_dict_t *) object2, new_val, old_val)))
                                                            {
                                                                modified = nyx_dict_set_alt((nyx_dict_t *) object2, "$", nyx_string_from(new_val), false);
                                                            }
                                                        }

                                                        break;

                                                    /*----------------------------------------------------------------*/

                                                    case 0xFEC07AA7:    // defLightVector
                                                        {
                                                            nyx_state_t old_val = nyx_str_to_state(nyx_string_get((nyx_string_t *) old_value));
                                                            nyx_state_t new_val = nyx_str_to_state(nyx_string_get((nyx_string_t *) new_value));

                                                            if((success = object2->in_callback._double == NULL || object2->in_callback._int(vector, (nyx_dict_t *) object2, new_val, old_val)))
                                                            {
                                                                modified = nyx_dict_set_alt((nyx_dict_t *) object2, "$", nyx_string_from(nyx_state_to_str(new_val)), false);
                                                            }
                                                        }

                                                        break;

                                                    /*----------------------------------------------------------------*/

                                                    case 0x17C598B1:    // defSwitchVector
                                                        {
                                                            nyx_onoff_t old_val = nyx_str_to_onoff(nyx_string_get((nyx_string_t *) old_value));
                                                            nyx_onoff_t new_val = nyx_str_to_onoff(nyx_string_get((nyx_string_t *) new_value));

                                                            if((success = object2->in_callback._double == NULL || object2->in_callback._int(vector, (nyx_dict_t *) object2, new_val, old_val)))
                                                            {
                                                                modified = nyx_dict_set_alt((nyx_dict_t *) object2, "$", nyx_string_from(nyx_onoff_to_str(new_val)), false);
                                                            }
                                                        }

                                                        break;

                                                    /*----------------------------------------------------------------*/

                                                    case 0x29BFE4D7:    // defBLOBVector
                                                        {
                                                            size_t size;
                                                            buff_t buff;

                                                            bool compress = internal_blob_is_compressed((nyx_dict_t *) object2);

                                                            nyx_string_get_buff((nyx_string_t *) new_value, &size, &buff, true, compress);

                                                            if((success = object2->in_callback._str == NULL || object2->in_callback._buffer(vector, (nyx_dict_t *) object2, size, buff)))
                                                            {
                                                                modified = nyx_dict_set_alt((nyx_dict_t *) object2, "$", nyx_string_from_buff(size, buff, true, compress), false);
                                                            }
                                                        }

                                                        break;

                                                    /*----------------------------------------------------------------*/

                                                    default:
                                                        NYX_LOG_ERROR("Invalid INDI / Nyx object");
                                                        continue;

                                                    /*----------------------------------------------------------------*/
                                                }

                                                /*--------------------------------------------------------------------*/

                                                if(success)
                                                {
                                                    str_t str = nyx_object_to_string(object2);
                                                    NYX_LOG_DEBUG("Updating (modified: %s) `%s::%s` with %s", modified ? "true" : "false", device1, name1, str);
                                                    nyx_memory_free(str);
                                                }

                                                /*--------------------------------------------------------------------*/

                                                vector_modified = vector_modified || modified;

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

                    if(vector->base.in_callback._vector != NULL) vector->base.in_callback._vector(vector, vector_modified);

                    nyx_object_notify(&vector->base);

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

static void _process_message(nyx_node_t *node, nyx_object_t *object)
{
    if(object->type == NYX_TYPE_DICT)
    {
        STR_t tag = nyx_dict_get_string((nyx_dict_t *) object, "<>");

        if(tag != NULL)
        {
            /**/ if(strcmp(tag, "getProperties") == 0)
            {
                nyx_node_lock(node);
                /**/    _get_properties_unsafe(node, (nyx_dict_t *) object);
                nyx_node_unlock(node);
            }
            else if(strcmp(tag, "enableBLOB") == 0)
            {
                nyx_node_lock(node);
                /**/    _enable_blob_unsafe(node, (nyx_dict_t *) object);
                nyx_node_unlock(node);
            }
            else if(strcmp(tag, "enableStream") == 0)
            {
                nyx_node_lock(node);
                /**/    _enable_stream_unsafe(node, (nyx_dict_t *) object);
                nyx_node_unlock(node);
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
                nyx_node_lock(node);
                /**/    _set_properties_unsafe(node, (nyx_dict_t *) object);
                nyx_node_unlock(node);
            }
        }
    }
}

/*--------------------------------------------------------------------------------------------------------------------*/

static size_t _tcp_handler(nyx_node_t *node, nyx_event_t event_type, const nyx_str_t payload)
{
    /*----------------------------------------------------------------------------------------------------------------*/
    /* NYX_EVENT_MSG                                                                                                  */
    /*----------------------------------------------------------------------------------------------------------------*/

    if(event_type == NYX_EVENT_MSG)
    {
        nyx_xml_stream_t xml_stream = NYX_XML_STREAM();

        if(nyx_xml_stream_detect_opening_tag(&xml_stream, payload.len, payload.buf))
        {
            if(nyx_xml_stream_detect_closing_tag(&xml_stream, payload.len, payload.buf))
            {
                /*----------------------------------------------------------------------------------------------------*/

                nyx_xmldoc_t *xmldoc = nyx_xmldoc_parse_buff(xml_stream.len, xml_stream.s_ptr);

                if(xmldoc != NULL)
                {
                    nyx_object_t *object = nyx_xmldoc_to_object(xmldoc);

                    if(object != NULL)
                    {
                        _process_message(node, object);

                        nyx_object_free(object);
                    }

                    nyx_xmldoc_free(xmldoc);
                }

                /*----------------------------------------------------------------------------------------------------*/

                return xml_stream.pos + xml_stream.len;

                /*----------------------------------------------------------------------------------------------------*/
            }
        }
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    return 0;
}

/*--------------------------------------------------------------------------------------------------------------------*/

static void _mqtt_handler(nyx_node_t *node, nyx_event_t event_type, const nyx_str_t event_topic, const nyx_str_t event_payload)
{
    /*----------------------------------------------------------------------------------------------------------------*/
    /* NYX_EVENT_OPEN                                                                                                 */
    /*----------------------------------------------------------------------------------------------------------------*/

    if(event_type == NYX_EVENT_OPEN)
    {
        for(size_t i = 0; i < sizeof(SPECIAL_TOPICS) / sizeof(nyx_str_t); i++)
        {
            str_t topic = nyx_memory_alloc(SPECIAL_TOPICS[i].len + node->node_id.len + 2);

            if(sprintf(topic, "%s/%s", SPECIAL_TOPICS[i].buf, node->node_id.buf) > 0)
            {
                NYX_LOG_INFO("Subscribing to `%s` and `%s` topics",
                     SPECIAL_TOPICS[i].buf,
                     /*---*/ topic /*---*/
                );

                internal_mqtt_sub(node, SPECIAL_TOPICS[i]);

                internal_mqtt_sub(node, nyx_str_s(topic));
            }

            nyx_memory_free(topic);
        }

        /*------------------------------------------------------------------------------------------------------------*/

        if(node->user_mqtt_handler != NULL)
        {
            node->user_mqtt_handler(
                node,
                NYX_EVENT_OPEN,
                0x00, NULL,
                0x00, NULL
            );
        }

        /*------------------------------------------------------------------------------------------------------------*/

        nyx_node_lock(node);
        /**/    _get_properties_unsafe(node, NULL);
        nyx_node_unlock(node);

        /*------------------------------------------------------------------------------------------------------------*/
    }

	/*----------------------------------------------------------------------------------------------------------------*/
	/* NYX_EVENT_MSG                                                                                                  */
	/*----------------------------------------------------------------------------------------------------------------*/

    else if(event_type == NYX_EVENT_MSG)
    {
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
                if(event_payload.len > 0 && event_payload.buf != NULL)
                {
                    /**/ if(nyx_startswith(event_topic, SPECIAL_TOPICS[1]))
                    {
                        /*--------------------------------------------------------------------------------------------*/
                        /* SET_MASTER_CLIENT                                                                          */
                        /*--------------------------------------------------------------------------------------------*/

                        nyx_memory_free(node->master_client_message.buf);

                        /*--------------------------------------------------------------------------------------------*/

                        node->master_client_message.buf = nyx_string_ndup(event_payload.buf, node->master_client_message.len = event_payload.len);

                        /*--------------------------------------------------------------------------------------------*/
                    }
                    else if(nyx_startswith(event_topic, SPECIAL_TOPICS[2]))
                    {
                        /*--------------------------------------------------------------------------------------------*/
                        /* JSON NEW XXX VECTOR                                                                        */
                        /*--------------------------------------------------------------------------------------------*/

                        nyx_object_t *object = nyx_object_parse_buff(event_payload.len, event_payload.buf);

                        if(object != NULL)
                        {
                            _process_message(node, object);

                            nyx_object_free(object);
                        }

                        /*--------------------------------------------------------------------------------------------*/
                    }
                    else if(nyx_startswith(event_topic, SPECIAL_TOPICS[3]))
                    {
                        /*--------------------------------------------------------------------------------------------*/
                        /* XML NEW XXX VECTOR                                                                         */
                        /*--------------------------------------------------------------------------------------------*/

                        nyx_xmldoc_t *xmldoc = nyx_xmldoc_parse_buff(event_payload.len, event_payload.buf);

                        if(xmldoc != NULL)
                        {
                            nyx_object_t *object = nyx_xmldoc_to_object(xmldoc);

                            if(object != NULL)
                            {
                                _process_message(node, object);

                                nyx_object_free(object);
                            }

                            nyx_xmldoc_free(xmldoc);
                        }

                        /*--------------------------------------------------------------------------------------------*/
                    }
                    else if(node->user_mqtt_handler != NULL)
                    {
                        /*--------------------------------------------------------------------------------------------*/
                        /* USER MESSAGE                                                                               */
                        /*--------------------------------------------------------------------------------------------*/

                        node->user_mqtt_handler(
                            node,
                            NYX_EVENT_MSG,
                            event_topic.len,
                            event_topic.buf,
                            event_payload.len,
                            event_payload.buf
                        );

                        /*--------------------------------------------------------------------------------------------*/
                    }
                }
            }
        }
    }

	/*----------------------------------------------------------------------------------------------------------------*/
}

/*--------------------------------------------------------------------------------------------------------------------*/

nyx_node_t *nyx_node_initialize(
    STR_t node_id,
    nyx_dict_t *vectors[],
    /**/
    __NULLABLE__ STR_t indi_url,
    /**/
    __NULLABLE__ STR_t mqtt_url,
    __NULLABLE__ STR_t mqtt_username,
    __NULLABLE__ STR_t mqtt_password,
    /**/
    __NULLABLE__ nyx_mqtt_handler_t mqtt_handler,
    /**/
    __NULLABLE__ STR_t redis_url,
    __NULLABLE__ STR_t redis_username,
    __NULLABLE__ STR_t redis_password,
    /**/
    int retry_ms,
    bool enable_xml
) {
    /*----------------------------------------------------------------------------------------------------------------*/
    /* ALLOCATE NODE                                                                                                  */
    /*----------------------------------------------------------------------------------------------------------------*/

    nyx_node_t *node = nyx_memory_alloc(sizeof(nyx_node_t));

    memset(node, 0x00, sizeof(nyx_node_t));

    /*----------------------------------------------------------------------------------------------------------------*/
    /* PATCH VECTORS                                                                                                  */
    /*----------------------------------------------------------------------------------------------------------------*/

    for(nyx_dict_t **vector_ptr = vectors; *vector_ptr != NULL; vector_ptr++)
    {
        nyx_dict_t *vector = *vector_ptr;

        /*------------------------------------------------------------------------------------------------------------*/

        nyx_dict_set_alt(vector, "@client", nyx_string_from(node_id), false);

        /*------------------------------------------------------------------------------------------------------------*/

        nyx_object_t *children = nyx_dict_get(vector, "children");

        if(children != NULL && children->type == NYX_TYPE_LIST)
        {
            int idx;
            nyx_object_t *vector_def;

            for(nyx_list_iter_t iter = NYX_LIST_ITER((nyx_list_t *) children); nyx_list_iterate(&iter, &idx, &vector_def);)
            {
                vector_def->out_callback = /**/ NULL /**/;

                vector_def->node = node;
            }
        }

        vector->base.out_callback = _out_callback;

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
        node->master_client_message = nyx_str_s(nyx_string_dup(NYX_ALL));

        node->master_client_topic = nyx_str_s(master_client_topic);
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    node->indi_url = indi_url;
    node->mqtt_url = mqtt_url;
    node->redis_url = redis_url;

    node->enable_xml = enable_xml;

    node->vectors = vectors;

    node->tcp_handler = _tcp_handler;
    node->mqtt_handler = _mqtt_handler;
    node->user_mqtt_handler = mqtt_handler;

    /*----------------------------------------------------------------------------------------------------------------*/
    /* INITIALIZE UNDERLYING STACK                                                                                    */
    /*----------------------------------------------------------------------------------------------------------------*/

    nyx_node_stack_initialize(node, mqtt_username, mqtt_password, redis_username, redis_password, retry_ms);

    /*----------------------------------------------------------------------------------------------------------------*/

    return node;
}

/*--------------------------------------------------------------------------------------------------------------------*/

void nyx_node_finalize(nyx_node_t *node, bool free_vectors)
{
    if(node != NULL)
    {
        /*------------------------------------------------------------------------------------------------------------*/
        /* FINALIZE UNDERLYING STACK                                                                                  */
        /*------------------------------------------------------------------------------------------------------------*/

        nyx_node_stack_finalize(node);

        /*------------------------------------------------------------------------------------------------------------*/
        /* FREE DEF VECTORS                                                                                           */
        /*------------------------------------------------------------------------------------------------------------*/

        if(free_vectors)
        {
            for(nyx_dict_t **vector_ptr = node->vectors; *vector_ptr != NULL; vector_ptr++)
            {
                nyx_dict_free(*vector_ptr);
            }
        }

        /*------------------------------------------------------------------------------------------------------------*/
        /* FREE NODE                                                                                                  */
        /*------------------------------------------------------------------------------------------------------------*/

        nyx_memory_free(node->master_client_message.buf);

        nyx_memory_free(node->master_client_topic.buf);

        /*------------------------------------------------------------------------------------------------------------*/

        nyx_memory_free(node);

        /*------------------------------------------------------------------------------------------------------------*/
    }
}

/*--------------------------------------------------------------------------------------------------------------------*/

void nyx_node_ping(nyx_node_t *node)
{
    if(node != NULL)
    {
        internal_mqtt_pub(node, nyx_str_s("nyx/ping/node"), node->node_id);

        internal_mqtt_pub(node, node->master_client_topic, node->master_client_message);
    }
}

/*--------------------------------------------------------------------------------------------------------------------*/

static void _device_onoff(nyx_node_t *node, STR_t device, __NULLABLE__ STR_t name, __NULLABLE__ STR_t message, nyx_onoff_t onoff)
{
    /*----------------------------------------------------------------------------------------------------------------*/

    if(node != NULL && device != NULL)
    {
        /*------------------------------------------------------------------------------------------------------------*/

        for(nyx_dict_t **vector_ptr = node->vectors; *vector_ptr != NULL; vector_ptr++)
        {
            nyx_dict_t *vector = *vector_ptr;

            /*--------------------------------------------------------------------------------------------------------*/

            STR_t device2 = nyx_dict_get_string(vector, "@device");
            STR_t name2 = nyx_dict_get_string(vector, "@name");

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
                    vector->base.flags |= NYX_FLAGS_DISABLED;
                    break;

                case NYX_ONOFF_ON:
                    vector->base.flags &= ~NYX_FLAGS_DISABLED;

                    _sub_object(node, (nyx_object_t *) vector);
                    break;
            }

            /*--------------------------------------------------------------------------------------------------------*/
        }

        /*------------------------------------------------------------------------------------------------------------*/

        if(onoff == NYX_ONOFF_OFF)
        {
            nyx_dict_t *del_property_new = nyx_del_property_new(device, name, message);

            _sub_object(node, (nyx_object_t *) del_property_new);

            nyx_dict_free(del_property_new);
        }

        /*------------------------------------------------------------------------------------------------------------*/
    }
}

/*--------------------------------------------------------------------------------------------------------------------*/

void nyx_node_enable(nyx_node_t *node, STR_t device, __NULLABLE__ STR_t name, __NULLABLE__ STR_t message)
{
    _device_onoff(node, device, name, message, NYX_ONOFF_ON);
}

/*--------------------------------------------------------------------------------------------------------------------*/

void nyx_node_disable(nyx_node_t *node, STR_t device, __NULLABLE__ STR_t name, __NULLABLE__ STR_t message)
{
    _device_onoff(node, device, name, message, NYX_ONOFF_OFF);
}

/*--------------------------------------------------------------------------------------------------------------------*/

void nyx_node_send_message(nyx_node_t *node, STR_t device, __NULLABLE__ STR_t message)
{
    if(node != NULL)
    {
        nyx_dict_t *dict = nyx_message_new(device, message);

        _sub_object(node, (nyx_object_t *) dict);

        nyx_dict_free(dict);
    }
}

/*--------------------------------------------------------------------------------------------------------------------*/

void nyx_node_send_del_property(nyx_node_t *node, STR_t device, __NULLABLE__ STR_t name, __NULLABLE__ STR_t message)
{
    if(node != NULL)
    {
        nyx_dict_t *dict = nyx_del_property_new(device, name, message);

        _sub_object(node, (nyx_object_t *) dict);

        nyx_dict_free(dict);
    }
}

/*--------------------------------------------------------------------------------------------------------------------*/
