/* NyxNode
 * Author: Jérôme ODIER <jerome.odier@lpsc.in2p3.fr>
 * SPDX-License-Identifier: GPL-2.0-only (Mongoose backend) or GPL-3.0+
 */

/*--------------------------------------------------------------------------------------------------------------------*/

#include <time.h>
#include <stdio.h>
#include <string.h>

#include "../nyx_node_internal.h"

/*--------------------------------------------------------------------------------------------------------------------*/
/* STATE                                                                                                              */
/*--------------------------------------------------------------------------------------------------------------------*/

STR_t nyx_state_to_str(nyx_state_t state)
{
    switch(state)
    {
        case NYX_STATE_IDLE:
            return "Idle";
        case NYX_STATE_OK:
            return "Ok";
        case NYX_STATE_BUSY:
            return "Busy";
        case NYX_STATE_ALERT:
            return "Alert";
    }

    NYX_LOG_FATAL("Internal error");
}

/*--------------------------------------------------------------------------------------------------------------------*/

nyx_state_t nyx_str_to_state(STR_t state)
{
    if(strcmp("Idle", state) == 0) {
        return NYX_STATE_IDLE;
    }
    if(strcmp("Ok", state) == 0) {
        return NYX_STATE_OK;
    }
    if(strcmp("Busy", state) == 0) {
        return NYX_STATE_BUSY;
    }
    if(strcmp("Alert", state) == 0) {
        return NYX_STATE_ALERT;
    }

    NYX_LOG_FATAL("Internal error");
}

/*--------------------------------------------------------------------------------------------------------------------*/
/* PERM                                                                                                               */
/*--------------------------------------------------------------------------------------------------------------------*/

STR_t nyx_perm_to_str(nyx_perm_t perm)
{
    switch(perm)
    {
        case NYX_PERM_RO:
            return "ro";
        case NYX_PERM_WO:
            return "wo";
        case NYX_PERM_RW:
            return "rw";
    }

    NYX_LOG_FATAL("Internal error");
}

/*--------------------------------------------------------------------------------------------------------------------*/

nyx_perm_t nyx_str_to_perm(STR_t perm)
{
    if(strcmp("ro", perm) == 0) {
        return NYX_PERM_RO;
    }
    if(strcmp("wo", perm) == 0) {
        return NYX_PERM_WO;
    }
    if(strcmp("rw", perm) == 0) {
        return NYX_PERM_RW;
    }

    NYX_LOG_FATAL("Internal error");
}

/*--------------------------------------------------------------------------------------------------------------------*/
/* RULE                                                                                                               */
/*--------------------------------------------------------------------------------------------------------------------*/

STR_t nyx_rule_to_str(nyx_rule_t rule)
{
    switch(rule)
    {
        case NYX_RULE_ONE_OF_MANY:
            return "OneOfMany";
        case NYX_RULE_AT_MOST_ONE:
            return "AtMostOne";
        case NYX_RULE_ANY_OF_MANY:
            return "AnyOfMany";
    }

    NYX_LOG_FATAL("Internal error");
}

/*--------------------------------------------------------------------------------------------------------------------*/

nyx_rule_t nyx_str_to_rule(STR_t rule)
{
    if(strcmp("OneOfMany", rule) == 0) {
        return NYX_RULE_ONE_OF_MANY;
    }
    if(strcmp("AtMostOne", rule) == 0) {
        return NYX_RULE_AT_MOST_ONE;
    }
    if(strcmp("AnyOfMany", rule) == 0) {
        return NYX_RULE_ANY_OF_MANY;
    }
    NYX_LOG_FATAL("Internal error");
}

/*--------------------------------------------------------------------------------------------------------------------*/
/* ONOFF                                                                                                              */
/*--------------------------------------------------------------------------------------------------------------------*/

STR_t nyx_onoff_to_str(nyx_onoff_t onoff)
{
    switch(onoff)
    {
        case NYX_ONOFF_ON:
            return "On";
        case NYX_ONOFF_OFF:
            return "Off";
    }

    NYX_LOG_FATAL("Internal error");
}

/*--------------------------------------------------------------------------------------------------------------------*/

nyx_onoff_t nyx_str_to_onoff(STR_t onoff)
{
    if(strcmp("On", onoff) == 0) {
        return NYX_ONOFF_ON;
    }
    if(strcmp("Off", onoff) == 0) {
        return NYX_ONOFF_OFF;
    }

    NYX_LOG_FATAL("Internal error");
}

/*--------------------------------------------------------------------------------------------------------------------*/
/* BLOB                                                                                                              */
/*--------------------------------------------------------------------------------------------------------------------*/

STR_t nyx_blob_state_to_str(nyx_blob_state_t blob)
{
    switch(blob)
    {
        case NYX_BLOB_STATE_DISABLED:
            return "Never";
        case NYX_BLOB_STATE_ENABLED:
            return "Also";
    }

    NYX_LOG_FATAL("Internal error");
}

/*--------------------------------------------------------------------------------------------------------------------*/

nyx_blob_state_t nyx_str_to_blob_state(STR_t blob)
{
    if(strcmp("Never", blob) == 0) {
        return NYX_BLOB_STATE_DISABLED;
    }
    if(strcmp("Also", blob) == 0
       ||
       strcmp("Only", blob) == 0
    ) {
        return NYX_BLOB_STATE_ENABLED;
    }

    NYX_LOG_FATAL("Internal error");
}

/*--------------------------------------------------------------------------------------------------------------------*/
/* STREAM                                                                                                             */
/*--------------------------------------------------------------------------------------------------------------------*/

STR_t nyx_stream_state_to_str(nyx_stream_state_t stream)
{
    switch(stream)
    {
        case NYX_STREAM_STATE_DISABLED:
            return "Never";
        case NYX_STREAM_STATE_ENABLED:
            return "Also";
    }

    NYX_LOG_FATAL("Internal error");
}

/*--------------------------------------------------------------------------------------------------------------------*/

nyx_stream_state_t nyx_str_to_stream_state(STR_t stream)
{
    if(strcmp("Never", stream) == 0) {
        return NYX_STREAM_STATE_DISABLED;
    }
    if(strcmp("Also", stream) == 0
       ||
       strcmp("Only", stream) == 0
    ) {
        return NYX_STREAM_STATE_ENABLED;
    }

    NYX_LOG_FATAL("Internal error");
}

/*--------------------------------------------------------------------------------------------------------------------*/
/* HELPERS                                                                                                            */
/*--------------------------------------------------------------------------------------------------------------------*/

bool internal_copy(nyx_dict_t *dst, const nyx_dict_t *src, STR_t key, bool notify)
{
    nyx_object_t *src_object = nyx_dict_get(src, key);

    if(src_object != NULL)
    {
        /*------------------------------------------------------------------------------------------------------------*/

        switch(src_object->type)
        {
            /*--------------------------------------------------------------------------------------------------------*/

            case NYX_TYPE_NULL:
                return src_object->type == NYX_TYPE_NULL;

            /*--------------------------------------------------------------------------------------------------------*/

            case NYX_TYPE_NUMBER:
                return nyx_dict_set_alt(dst, key, nyx_number_from(nyx_number_get((nyx_number_t *) src_object)), notify);

            /*--------------------------------------------------------------------------------------------------------*/

            case NYX_TYPE_BOOLEAN:
                return nyx_dict_set_alt(dst, key, nyx_boolean_from(nyx_boolean_get((nyx_boolean_t *) src_object)), notify);

            /*--------------------------------------------------------------------------------------------------------*/

            case NYX_TYPE_STRING:
                return nyx_dict_set_alt(dst, key, nyx_string_from_dup(nyx_string_get((nyx_string_t *) src_object)), notify);

            /*--------------------------------------------------------------------------------------------------------*/

            default:
                NYX_LOG_ERROR("Invalid object");
                break;

            /*--------------------------------------------------------------------------------------------------------*/
        }

        /*------------------------------------------------------------------------------------------------------------*/
    }

    return false;
}

/*--------------------------------------------------------------------------------------------------------------------*/

int internal_get_timestamp(size_t size, str_t str)
{
    time_t now = time(NULL);

    struct tm *tm_now = localtime(&now);

    return snprintf(
        str,
        size,
        "%04d-%02d-%02dT%02d:%02d:%02d",
        tm_now->tm_year + 1900,
        tm_now->tm_mon + 1,
        tm_now->tm_mday,
        tm_now->tm_hour,
        tm_now->tm_min,
        tm_now->tm_sec
    );
}

/*--------------------------------------------------------------------------------------------------------------------*/

void internal_set_opts(nyx_dict_t *dict, __NULLABLE__ const nyx_opts_t *opts)
{
    /*----------------------------------------------------------------------------------------------------------------*/

    char timestamp[36];

    internal_get_timestamp(sizeof(timestamp), timestamp);

    nyx_dict_set(dict, "@timestamp", nyx_string_from_dup(timestamp));

    /*----------------------------------------------------------------------------------------------------------------*/

    STR_t group = "Main";

    if(opts != NULL)
    {
        /*------------------------------------------------------------------------------------------------------------*/

        if(opts->group != NULL && opts->group[0] != '\0')
        {
            group = opts->group;
        }

        /*------------------------------------------------------------------------------------------------------------*/

        if(opts->label != NULL) {
            nyx_dict_set(dict, "@label", nyx_string_from_dup(opts->label));
        }

        if(opts->hints != NULL) {
            nyx_dict_set(dict, "@hints", nyx_string_from_dup(opts->hints));
        }

        if(opts->message != NULL) {
            nyx_dict_set(dict, "@message", nyx_string_from_dup(opts->message));
        }

        if(opts->timeout > 0.00) {
            nyx_dict_set(dict, "@timeout", nyx_number_from(opts->timeout));
        }

        /*------------------------------------------------------------------------------------------------------------*/
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    nyx_dict_set(dict, "@group", nyx_string_from_dup(group));

    /*----------------------------------------------------------------------------------------------------------------*/
}

/*--------------------------------------------------------------------------------------------------------------------*/

bool internal_blob_is_compressed(const nyx_dict_t *def)
{
    nyx_string_t *format = (nyx_string_t *) nyx_dict_get(def, "@format");

    return format != NULL && format->length > 2 && format->value[format->length - 2] == '.' && format->value[format->length - 1] == 'z';
}

/*--------------------------------------------------------------------------------------------------------------------*/

static void internal_copy_blob(nyx_dict_t *dst_dict, const nyx_dict_t *src_dict, bool notify)
{
    /*----------------------------------------------------------------------------------------------------------------*/

    internal_copy(dst_dict, src_dict, "@size", notify);
    internal_copy(dst_dict, src_dict, "@format", notify);

    /*----------------------------------------------------------------------------------------------------------------*/

    nyx_object_t *src_payload = nyx_dict_get(src_dict, "$");

    if(src_payload != NULL && src_payload->type == NYX_TYPE_STRING)
    {
        /*------------------------------------------------------------------------------------------------------------*/

        size_t src_size;
        buff_t src_buff;

        nyx_string_get_buff((nyx_string_t *) src_payload, &src_size, &src_buff);

        /*------------------------------------------------------------------------------------------------------------*/

        size_t dst_len;
        str_t dst_str;

        if(internal_blob_is_compressed(src_dict)) {
            dst_str = nyx_zlib_base64_deflate(&dst_len, src_size, src_buff);
        }
        else {
            dst_str = nyx_base64_encode(&dst_len, src_size, src_buff);
        }

        /*------------------------------------------------------------------------------------------------------------*/

        nyx_dict_set_alt(dst_dict, "$", nyx_string_from_buff_managed(dst_len, dst_str), notify);

        /*------------------------------------------------------------------------------------------------------------*/
    }

    /*----------------------------------------------------------------------------------------------------------------*/
}

/*--------------------------------------------------------------------------------------------------------------------*/

nyx_dict_t *internal_prop_to_set_vector(const nyx_dict_t *vector, STR_t set_tag, STR_t one_tag)
{
    /*----------------------------------------------------------------------------------------------------------------*/

    nyx_dict_t *result = nyx_dict_new();

    nyx_list_t *children = nyx_list_new();

    /*----------------------------------------------------------------------------------------------------------------*/

    nyx_dict_set(result, "<>", nyx_string_from_dup(set_tag));

    internal_copy(result, vector, "@client", false);
    internal_copy(result, vector, "@device", false);
    internal_copy(result, vector, "@name", false);
    internal_copy(result, vector, "@state", false);
    internal_copy(result, vector, "@timeout", false);
    internal_copy(result, vector, "@timestamp", false);
    internal_copy(result, vector, "@message", false);

    nyx_dict_set(result, "children", children);

    /*----------------------------------------------------------------------------------------------------------------*/

    size_t idx;

    nyx_object_t *object;

    nyx_object_t *list = nyx_dict_get(vector, "children");

    if(list != NULL && list->type == NYX_TYPE_LIST)
    {
        for(nyx_list_iter_t iter = NYX_LIST_ITER(list); nyx_list_iterate(&iter, &idx, &object);)
        {
            if(object->type == NYX_TYPE_DICT)
            {
                /*----------------------------------------------------------------------------------------------------*/

                nyx_dict_t *src_dict = (nyx_dict_t *) object, *dst_dict = nyx_dict_new();

                /*----------------------------------------------------------------------------------------------------*/

                nyx_dict_set_alt(dst_dict, "<>", nyx_string_from_dup(one_tag), false);

                internal_copy(dst_dict, src_dict, "@name", false);

                if(strcmp(one_tag, "oneBLOB") == 0) {
                    internal_copy_blob(dst_dict, src_dict, false);
                }
                else {
                    internal_copy(dst_dict, src_dict, "$", false);
                }

                /*----------------------------------------------------------------------------------------------------*/

                nyx_list_push(children, dst_dict);

                /*----------------------------------------------------------------------------------------------------*/
            }
        }
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    return result;
}

/*--------------------------------------------------------------------------------------------------------------------*/
