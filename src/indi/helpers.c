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
    /**/ if(strcmp("Idle", state) == 0) {
        return NYX_STATE_IDLE;
    }
    else if(strcmp("Ok", state) == 0) {
        return NYX_STATE_OK;
    }
    else if(strcmp("Busy", state) == 0) {
        return NYX_STATE_BUSY;
    }
    else if(strcmp("Alert", state) == 0) {
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
    /**/ if(strcmp("ro", perm) == 0) {
        return NYX_PERM_RO;
    }
    else if(strcmp("wo", perm) == 0) {
        return NYX_PERM_WO;
    }
    else if(strcmp("rw", perm) == 0) {
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
    /**/ if(strcmp("OneOfMany", rule) == 0) {
        return NYX_RULE_ONE_OF_MANY;
    }
    else if(strcmp("AtMostOne", rule) == 0) {
        return NYX_RULE_AT_MOST_ONE;
    }
    else if(strcmp("AnyOfMany", rule) == 0) {
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
    /**/ if(strcmp("On", onoff) == 0) {
        return NYX_ONOFF_ON;
    }
    else if(strcmp("Off", onoff) == 0) {
        return NYX_ONOFF_OFF;
    }

    NYX_LOG_FATAL("Internal error");
}

/*--------------------------------------------------------------------------------------------------------------------*/
/* BLOB                                                                                                              */
/*--------------------------------------------------------------------------------------------------------------------*/

STR_t nyx_blob_to_str(nyx_blob_t blob)
{
    switch(blob)
    {
        case NYX_BLOB_NEVER:
            return "Never";
        case NYX_BLOB_ALSO:
            return "Also";
        case NYX_BLOB_ONLY:
            return "Only";
    }

    NYX_LOG_FATAL("Internal error");
}

/*--------------------------------------------------------------------------------------------------------------------*/

nyx_blob_t nyx_str_to_blob(STR_t blob)
{
    /**/ if(strcmp("Never", blob) == 0) {
        return NYX_BLOB_NEVER;
    }
    else if(strcmp("Also", blob) == 0) {
        return NYX_BLOB_ALSO;
    }
    else if(strcmp("Only", blob) == 0) {
        return NYX_BLOB_ONLY;
    }

    NYX_LOG_FATAL("Internal error");
}

/*--------------------------------------------------------------------------------------------------------------------*/
/* STREAM                                                                                                             */
/*--------------------------------------------------------------------------------------------------------------------*/

STR_t nyx_stream_to_str(nyx_stream_t stream)
{
    switch(stream)
    {
        case NYX_STREAM_NEVER:
            return "Never";
        case NYX_STREAM_ALSO:
            return "Also";
        case NYX_STREAM_ONLY:
            return "Only";
    }

    NYX_LOG_FATAL("Internal error");
}

/*--------------------------------------------------------------------------------------------------------------------*/

nyx_stream_t nyx_str_to_stream(STR_t stream)
{
    /**/ if(strcmp("Never", stream) == 0) {
        return NYX_STREAM_NEVER;
    }
    else if(strcmp("Also", stream) == 0) {
        return NYX_STREAM_ALSO;
    }
    else if(strcmp("Only", stream) == 0) {
        return NYX_STREAM_ONLY;
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
                return nyx_dict_set_alt(dst, key, nyx_string_from(nyx_string_get((nyx_string_t *) src_object)), notify);

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

int internal_get_timestamp(str_t buff, size_t size)
{
    time_t now = time(NULL);

    struct tm *tm_now = localtime(&now);

    return snprintf(
        buff,
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

    internal_get_timestamp(timestamp, sizeof(timestamp));

    nyx_dict_set(dict, "@timestamp", nyx_string_from(timestamp));

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
            nyx_dict_set(dict, "@label", nyx_string_from(opts->label));
        }

        if(opts->hints != NULL) {
            nyx_dict_set(dict, "@hints", nyx_string_from(opts->hints));
        }

        if(opts->timeout > 0.00) {
            nyx_dict_set(dict, "@timeout", nyx_number_from(opts->timeout));
        }

        if(opts->message != NULL) {
            nyx_dict_set(dict, "@message", nyx_string_from(opts->message));
        }

        /*------------------------------------------------------------------------------------------------------------*/
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    nyx_dict_set(dict, "@group", nyx_string_from(group));

    /*----------------------------------------------------------------------------------------------------------------*/
}

/*--------------------------------------------------------------------------------------------------------------------*/

nyx_dict_t *internal_def_to_set(const nyx_dict_t *def_vector, STR_t set_tag, STR_t one_tag)
{
    /*----------------------------------------------------------------------------------------------------------------*/

    nyx_dict_t *result = nyx_dict_new();

    nyx_list_t *children = nyx_list_new();

    /*----------------------------------------------------------------------------------------------------------------*/

    nyx_dict_set(result, "<>", nyx_string_from(set_tag));

    internal_copy(result, def_vector, "@client", false);
    internal_copy(result, def_vector, "@device", false);
    internal_copy(result, def_vector, "@name", false);
    internal_copy(result, def_vector, "@state", false);
    internal_copy(result, def_vector, "@timeout", false);
    internal_copy(result, def_vector, "@timestamp", false);
    internal_copy(result, def_vector, "@message", false);

    nyx_dict_set(result, "children", children);

    /*----------------------------------------------------------------------------------------------------------------*/

    int idx;

    nyx_object_t *object;

    nyx_object_t *list = nyx_dict_get(def_vector, "children");

    if(list != NULL)
    {
        for(nyx_list_iter_t iter = NYX_LIST_ITER(list); nyx_list_iterate(&iter, &idx, &object);)
        {
            if(object->type == NYX_TYPE_DICT)
            {
                /*----------------------------------------------------------------------------------------------------*/

                nyx_dict_t *src_dict = (nyx_dict_t *) object, *dst_dict = nyx_dict_new();

                /*----------------------------------------------------------------------------------------------------*/

                nyx_dict_set_alt(dst_dict, "<>", nyx_string_from(one_tag), false);

                internal_copy(dst_dict, src_dict,   "$"  , false);
                internal_copy(dst_dict, src_dict, "@name", false);

                if(strcmp(one_tag, "oneBLOB") == 0)
                {
                    /*------------------------------------------------------------------------------------------------*/

                    internal_copy(dst_dict, src_dict, "@format", false);

                    /*------------------------------------------------------------------------------------------------*/

                    nyx_object_t *payload = nyx_dict_get(src_dict, "$");

                    if(payload != NULL && payload->type == NYX_TYPE_STRING)
                    {
                        nyx_dict_set_alt(dst_dict, "@size", nyx_number_from(nyx_string_raw_size((nyx_string_t *) payload)), false);
                    }

                    /*------------------------------------------------------------------------------------------------*/
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

bool nyx_glob_is_compressed(const nyx_dict_t *def)
{
    nyx_string_t *format = (nyx_string_t *) nyx_dict_get(def, "@format");

    return format != NULL && format->raw_size > 2 && strcmp(format->value + format->raw_size - 2, ".z") == 0;
}

/*--------------------------------------------------------------------------------------------------------------------*/
