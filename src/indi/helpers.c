/*--------------------------------------------------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../indi_node_internal.h"

/*--------------------------------------------------------------------------------------------------------------------*/
/* STATE                                                                                                              */
/*--------------------------------------------------------------------------------------------------------------------*/

STR_t indi_state_to_str(indi_state_t state)
{
    switch(state)
    {
        case INDI_STATE_IDLE:
            return "Idle";
        case INDI_STATE_OK:
            return "Ok";
        case INDI_STATE_BUSY:
            return "Busy";
        case INDI_STATE_ALERT:
            return "Alert";
    }

    fprintf(stderr, "Internal error in `indi_state_to_str`\n");
    fflush(stderr);
    exit(1);
}

/*--------------------------------------------------------------------------------------------------------------------*/

indi_state_t indi_str_to_state(STR_t state)
{
    /**/ if(strcmp("Idle", state) == 0) {
        return INDI_STATE_IDLE;
    }
    else if(strcmp("Ok", state) == 0) {
        return INDI_STATE_OK;
    }
    else if(strcmp("Busy", state) == 0) {
        return INDI_STATE_BUSY;
    }
    else if(strcmp("Alert", state) == 0) {
        return INDI_STATE_ALERT;
    }

    fprintf(stderr, "Internal error in `indi_str_to_state`\n");
    fflush(stderr);
    exit(1);
}

/*--------------------------------------------------------------------------------------------------------------------*/
/* PERM                                                                                                               */
/*--------------------------------------------------------------------------------------------------------------------*/

STR_t indi_perm_to_str(indi_perm_t perm)
{
    switch(perm)
    {
        case INDI_PERM_RO:
            return "ro";
        case INDI_PERM_WO:
            return "wo";
        case INDI_PERM_RW:
            return "rw";
    }

    fprintf(stderr, "Internal error in `indi_perm_to_str`\n");
    fflush(stderr);
    exit(1);
}

/*--------------------------------------------------------------------------------------------------------------------*/

indi_perm_t indi_str_to_perm(STR_t perm)
{
    /**/ if(strcmp("ro", perm) == 0) {
        return INDI_PERM_RO;
    }
    else if(strcmp("wo", perm) == 0) {
        return INDI_PERM_WO;
    }
    else if(strcmp("rw", perm) == 0) {
        return INDI_PERM_RW;
    }

    fprintf(stderr, "Internal error in `indi_str_to_perm`\n");
    fflush(stderr);
    exit(1);
}

/*--------------------------------------------------------------------------------------------------------------------*/
/* RULE                                                                                                               */
/*--------------------------------------------------------------------------------------------------------------------*/

STR_t indi_rule_to_str(indi_rule_t rule)
{
    switch(rule)
    {
        case INDI_RULE_ONE_OF_MANY:
            return "OneOfMany";
        case INDI_RULE_AT_MOST_ONE:
            return "AtMostOne";
        case INDI_RULE_ANY_OF_MANY:
            return "AnyOfMany";
    }

    fprintf(stderr, "Internal error in `indi_rule_to_str`\n");
    fflush(stderr);
    exit(1);
}

/*--------------------------------------------------------------------------------------------------------------------*/

indi_rule_t indi_str_to_rule(STR_t rule)
{
    /**/ if(strcmp("OneOfMany", rule) == 0) {
        return INDI_RULE_ONE_OF_MANY;
    }
    else if(strcmp("AtMostOne", rule) == 0) {
        return INDI_RULE_AT_MOST_ONE;
    }
    else if(strcmp("AnyOfMany", rule) == 0) {
        return INDI_RULE_ANY_OF_MANY;
    }

    fprintf(stderr, "Internal error in `indi_str_to_rule`\n");
    fflush(stderr);
    exit(1);
}

/*--------------------------------------------------------------------------------------------------------------------*/
/* ONOFF                                                                                                              */
/*--------------------------------------------------------------------------------------------------------------------*/

STR_t indi_onoff_to_str(indi_onoff_t onoff)
{
    switch(onoff)
    {
        case INDI_ONOFF_ON:
            return "On";
        case INDI_ONOFF_OFF:
            return "Off";
    }

    fprintf(stderr, "Internal error in `indi_onoff_to_str`\n");
    fflush(stderr);
    exit(1);
}

/*--------------------------------------------------------------------------------------------------------------------*/

indi_onoff_t indi_str_to_onoff(STR_t onoff)
{
    /**/ if(strcmp("On", onoff) == 0) {
        return INDI_ONOFF_ON;
    }
    else if(strcmp("Off", onoff) == 0) {
        return INDI_ONOFF_OFF;
    }

    fprintf(stderr, "Internal error in `indi_str_to_onoff`\n");
    fflush(stderr);
    exit(1);
}

/*--------------------------------------------------------------------------------------------------------------------*/
/* BLOB                                                                                                              */
/*--------------------------------------------------------------------------------------------------------------------*/

STR_t indi_blob_to_str(indi_blob_t blob)
{
    switch(blob)
    {
        case INDI_BLOB_NEVER:
            return "Never";
        case INDI_BLOB_ALSO:
            return "Also";
        case INDI_BLOB_ONLY:
            return "Only";
    }

    fprintf(stderr, "Internal error in `indi_blob_to_str`\n");
    fflush(stderr);
    exit(1);
}

/*--------------------------------------------------------------------------------------------------------------------*/

indi_blob_t indi_str_to_blob(STR_t blob)
{
    /**/ if(strcmp("Never", blob) == 0) {
        return INDI_BLOB_NEVER;
    }
    else if(strcmp("Also", blob) == 0) {
        return INDI_BLOB_ALSO;
    }
    else if(strcmp("Only", blob) == 0) {
        return INDI_BLOB_ONLY;
    }

    fprintf(stderr, "Internal error in `indi_str_to_blob`\n");
    fflush(stderr);
    exit(1);
}

/*--------------------------------------------------------------------------------------------------------------------*/
/* HELPERS                                                                                                            */
/*--------------------------------------------------------------------------------------------------------------------*/

bool internal_copy_entry(indi_dict_t *dst, const indi_dict_t *src, STR_t key)
{
    indi_object_t *src_object = indi_dict_get(src, key);

    if(src_object != NULL)
    {
        indi_object_t *dst_object = indi_dict_get(dst, key);

        /*------------------------------------------------------------------------------------------------------------*/

        switch(src_object->type)
        {
            /*--------------------------------------------------------------------------------------------------------*/

            case INDI_TYPE_NUMBER:
            {
                double src_value = indi_number_get((indi_number_t *) src_object);

                if(dst_object != NULL && dst_object->type == src_object->type)
                {
                    double dst_value = indi_number_get((indi_number_t *) dst_object);

                    if(dst_value != src_value)
                    {
                        indi_dict_set(dst, key, indi_number_from(src_value));

                        return true;
                    }
                }
                else
                {
                    indi_dict_set(dst, key, indi_number_from(src_value));

                    return true;
                }

                break;
            }

            /*--------------------------------------------------------------------------------------------------------*/

            case INDI_TYPE_STRING:
            {
                STR_t src_value = indi_string_get((indi_string_t *) src_object);

                if(dst_object != NULL && dst_object->type == src_object->type)
                {
                    STR_t dst_value = indi_string_get((indi_string_t *) dst_object);

                    if(strcmp(dst_value, src_value) != 0)
                    {
                        indi_dict_set(dst, key, indi_string_from(src_value));

                        return true;
                    }
                }
                else
                {
                    indi_dict_set(dst, key, indi_string_from(src_value));

                    return true;
                }

                break;
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

void internal_set_opts(indi_dict_t *dict, indi_opt_t *opt)
{
    if(opt != NULL)
    {
        /*------------------------------------------------------------------------------------------------------------*/

        if(opt->label != NULL) {
            indi_dict_set(dict, "@label", indi_string_from(opt->label));
        }

        if(opt->group != NULL) {
            indi_dict_set(dict, "@group", indi_string_from(opt->group));
        }

        if(opt->driver != NULL) {
            indi_dict_set(dict, "@driver", indi_string_from(opt->driver));
        }

        if(opt->timeout > 0.00) {
            indi_dict_set(dict, "@timeout", indi_number_from(opt->timeout));
        }

        if(opt->timestamp != NULL) {
            indi_dict_set(dict, "@timestamp", indi_string_from(opt->timestamp));
        }

        if(opt->message != NULL) {
            indi_dict_set(dict, "@message", indi_string_from(opt->message));
        }

        /*------------------------------------------------------------------------------------------------------------*/
    }
}

/*--------------------------------------------------------------------------------------------------------------------*/

indi_dict_t *internal_xxx_set_vector_new(const indi_dict_t *def_vector, STR_t set_tag_name, STR_t one_tag_name)
{
    /*----------------------------------------------------------------------------------------------------------------*/

    indi_dict_t *result = indi_dict_new();

    indi_list_t *children = indi_list_new();

    /*----------------------------------------------------------------------------------------------------------------*/

    indi_dict_set(result, "<>", indi_string_from(set_tag_name));

    internal_copy_entry(result, def_vector, "@client");
    internal_copy_entry(result, def_vector, "@device");
    internal_copy_entry(result, def_vector, "@name");
    internal_copy_entry(result, def_vector, "@state");
    internal_copy_entry(result, def_vector, "@timeout");
    internal_copy_entry(result, def_vector, "@timestamp");
    internal_copy_entry(result, def_vector, "@message");

    indi_dict_set(result, "children", children);

    /*----------------------------------------------------------------------------------------------------------------*/

    int idx;

    indi_object_t *object;

    indi_object_t *list = indi_dict_get(def_vector, "children");

    for(indi_list_iter_t iter = INDI_LIST_ITER(list); indi_list_iterate(&iter, &idx, &object);)
    {
        /*------------------------------------------------------------------------------------------------------------*/

        indi_dict_t *dict = indi_dict_new();

        indi_dict_set(dict, "<>", indi_string_from(one_tag_name));

        internal_copy_entry(dict, (indi_dict_t *) object, "$");
        internal_copy_entry(dict, (indi_dict_t *) object, "@name");
        internal_copy_entry(dict, (indi_dict_t *) object, "@size");
        internal_copy_entry(dict, (indi_dict_t *) object, "@format");

        indi_list_push(children, dict);

        /*------------------------------------------------------------------------------------------------------------*/
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    return result;
}

/*--------------------------------------------------------------------------------------------------------------------*/
