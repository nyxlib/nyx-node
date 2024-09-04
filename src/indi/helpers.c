/*--------------------------------------------------------------------------------------------------------------------*/

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
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

    fprintf(stderr, "Internal error in `nyx_state_to_str`\n");
    fflush(stderr);
    exit(1);
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

    fprintf(stderr, "Internal error in `nyx_str_to_state`\n");
    fflush(stderr);
    exit(1);
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

    fprintf(stderr, "Internal error in `nyx_perm_to_str`\n");
    fflush(stderr);
    exit(1);
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

    fprintf(stderr, "Internal error in `nyx_str_to_perm`\n");
    fflush(stderr);
    exit(1);
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

    fprintf(stderr, "Internal error in `nyx_rule_to_str`\n");
    fflush(stderr);
    exit(1);
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

    fprintf(stderr, "Internal error in `nyx_str_to_rule`\n");
    fflush(stderr);
    exit(1);
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

    fprintf(stderr, "Internal error in `nyx_onoff_to_str`\n");
    fflush(stderr);
    exit(1);
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

    fprintf(stderr, "Internal error in `nyx_str_to_onoff`\n");
    fflush(stderr);
    exit(1);
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

    fprintf(stderr, "Internal error in `nyx_blob_to_str`\n");
    fflush(stderr);
    exit(1);
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

    fprintf(stderr, "Internal error in `nyx_str_to_blob`\n");
    fflush(stderr);
    exit(1);
}

/*--------------------------------------------------------------------------------------------------------------------*/
/* HELPERS                                                                                                            */
/*--------------------------------------------------------------------------------------------------------------------*/

void internal_mask(nyx_dict_t **def_vectors, STR_t device, __NULLABLE__ STR_t name, uint32_t mask, bool clear)
{
    /*----------------------------------------------------------------------------------------------------------------*/

    if(device != NULL)
    {
        /*------------------------------------------------------------------------------------------------------------*/

        for(nyx_dict_t **def_vector_ptr = def_vectors; *def_vector_ptr != NULL; def_vector_ptr++)
        {
            nyx_dict_t *def_vector = *def_vector_ptr;

            /*--------------------------------------------------------------------------------------------------------*/

            STR_t device2 = nyx_dict_get_string(def_vector, "@device");
            STR_t name2 = nyx_dict_get_string(def_vector, "@name");

            /*--------------------------------------------------------------------------------------------------------*/

            if(device2 != NULL)
            {
                /*----------------------------------------------------------------------------------------------------*/

                if((strcmp(device, device2) != 0)
                   ||
                   (name != NULL && name2 != NULL && strcmp(name, name2) != 0)
                ) {
                    continue;
                }

                /*----------------------------------------------------------------------------------------------------*/

                if(clear) {
                    def_vector->base.flags &= ~NYX_FLAGS_XXXX_DISABLED;
                }
                else {
                    def_vector->base.flags |= NYX_FLAGS_XXXX_DISABLED;
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

bool internal_copy_entry(nyx_dict_t *dst, const nyx_dict_t *src, STR_t key, bool notify)
{
    nyx_object_t *src_object = nyx_dict_get(src, key);

    if(src_object != NULL)
    {
        /*------------------------------------------------------------------------------------------------------------*/

        switch(src_object->type)
        {
            /*--------------------------------------------------------------------------------------------------------*/

            case NYX_TYPE_NUMBER:
            {
                double src_value = nyx_number_get((nyx_number_t *) src_object);

                return nyx_dict_set2(dst, key, nyx_number_from(src_value), notify);
            }

            /*--------------------------------------------------------------------------------------------------------*/

            case NYX_TYPE_BOOLEAN:
            {
                bool src_value = nyx_boolean_get((nyx_boolean_t *) src_object);

                return nyx_dict_set2(dst, key, nyx_boolean_from(src_value), notify);
            }

            /*--------------------------------------------------------------------------------------------------------*/

            case NYX_TYPE_STRING:
            {
                STR_t src_value = nyx_string_get((nyx_string_t *) src_object);

                return nyx_dict_set2(dst, key, nyx_string_from(src_value), notify);
            }

            /*--------------------------------------------------------------------------------------------------------*/

            default:
                fprintf(stderr, "Internal error in `internal_copy_entry`\n");
                fflush(stderr);
                break;

            /*--------------------------------------------------------------------------------------------------------*/
        }

        /*------------------------------------------------------------------------------------------------------------*/
    }

    return false;
}

/*--------------------------------------------------------------------------------------------------------------------*/

void internal_get_timestamp(str_t timestamp_buff, size_t timestamp_size)
{
    time_t now = time(NULL);

    struct tm *tm_now = localtime(&now);

    snprintf(
        timestamp_buff,
        timestamp_size,
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

void internal_set_opts(nyx_dict_t *dict, nyx_opts_t *opts) {
    /*----------------------------------------------------------------------------------------------------------------*/

    char timestamp[32];

    internal_get_timestamp(timestamp, sizeof(timestamp));

    nyx_dict_set(dict, "@timestamp", nyx_string_from(timestamp));

    /*----------------------------------------------------------------------------------------------------------------*/

    STR_t group;

    if(opts == NULL)
    {
        group = "Main";
    }
    else
    {
        /*------------------------------------------------------------------------------------------------------------*/

        group = opts->group != NULL ? opts->group : "Main";

        /*------------------------------------------------------------------------------------------------------------*/

        if(opts->label != NULL) {
            nyx_dict_set(dict, "@label", nyx_string_from(opts->label));
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

nyx_dict_t *internal_xxxx_set_vector_new(const nyx_dict_t *def_vector, STR_t set_tagname, STR_t one_tagname)
{
    /*----------------------------------------------------------------------------------------------------------------*/

    nyx_dict_t *result = nyx_dict_new();

    nyx_list_t *children = nyx_list_new();

    /*----------------------------------------------------------------------------------------------------------------*/

    nyx_dict_set(result, "<>", nyx_string_from(set_tagname));

    internal_copy_entry(result, def_vector, "@client", true);
    internal_copy_entry(result, def_vector, "@device", true);
    internal_copy_entry(result, def_vector, "@name", true);
    internal_copy_entry(result, def_vector, "@state", true);
    internal_copy_entry(result, def_vector, "@timeout", true);
    internal_copy_entry(result, def_vector, "@timestamp", true);
    internal_copy_entry(result, def_vector, "@message", true);

    nyx_dict_set(result, "children", children);

    /*----------------------------------------------------------------------------------------------------------------*/

    int idx;

    nyx_object_t *object;

    nyx_object_t *list = nyx_dict_get(def_vector, "children");

    for(nyx_list_iter_t iter = NYX_LIST_ITER(list); nyx_list_iterate(&iter, &idx, &object);)
    {
        /*------------------------------------------------------------------------------------------------------------*/

        nyx_dict_t *dict = nyx_dict_new();

        /*------------------------------------------------------------------------------------------------------------*/

        nyx_dict_set(dict, "<>", nyx_string_from(one_tagname));

        internal_copy_entry(dict, (nyx_dict_t *) object, "$", true);
        internal_copy_entry(dict, (nyx_dict_t *) object, "@name", true);

        if(strcmp(one_tagname, "oneBLOB") == 0)
        {
            internal_copy_entry(dict, (nyx_dict_t *) object, "@size", true);
            internal_copy_entry(dict, (nyx_dict_t *) object, "@format", true);
        }

        /*------------------------------------------------------------------------------------------------------------*/

        nyx_list_push(children, dict);

        /*------------------------------------------------------------------------------------------------------------*/
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    return result;
}

/*--------------------------------------------------------------------------------------------------------------------*/
