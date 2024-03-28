/*--------------------------------------------------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../indi_base_internal.h"

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
/* HELPERS                                                                                                            */
/*--------------------------------------------------------------------------------------------------------------------*/

void internal_set_opts(indi_dict_t *dict, indi_opt_t *opt)
{
    if(opt != NULL)
    {
        /*------------------------------------------------------------------------------------------------------------*/

        if(opt->label != NULL) {
            indi_dict_set(dict, "@label", indi_string_from(opt->label));
        }

        /*------------------------------------------------------------------------------------------------------------*/

        if(opt->group != NULL) {
            indi_dict_set(dict, "@group", indi_string_from(opt->group));
        }

        /*------------------------------------------------------------------------------------------------------------*/

        if(opt->timeout > 0.00) {
            indi_dict_set(dict, "@timeout", indi_number_from(opt->timeout));
        }

        /*------------------------------------------------------------------------------------------------------------*/

        if(opt->timestamp != NULL) {
            indi_dict_set(dict, "@timestamp", indi_string_from(opt->timestamp));
        }

        /*------------------------------------------------------------------------------------------------------------*/

        if(opt->message != NULL) {
            indi_dict_set(dict, "@message", indi_string_from(opt->message));
        }

        /*------------------------------------------------------------------------------------------------------------*/
    }
}

/*--------------------------------------------------------------------------------------------------------------------*/

void internal_copy_entry(indi_dict_t *dst, indi_dict_t *src, STR_t key)
{
    indi_object_t *object = indi_dict_get(src, key);

    if(object != NULL)
    {
        switch(object->type)
        {
            /*--------------------------------------------------------------------------------------------------------*/

            case INDI_TYPE_NUMBER:
                indi_dict_set(
                    dst, key,
                    indi_number_from(indi_number_get((indi_number_t *) object))
                );
                break;

            /*--------------------------------------------------------------------------------------------------------*/

            case INDI_TYPE_STRING:
                indi_dict_set(
                    dst, key,
                    indi_string_from(indi_string_get((indi_string_t *) object))
                );
                break;

            /*--------------------------------------------------------------------------------------------------------*/

            default:
                fprintf(stderr, "\n");
                fflush(stderr);
                break;

            /*--------------------------------------------------------------------------------------------------------*/
        }
    }
}

/*--------------------------------------------------------------------------------------------------------------------*/

indi_dict_t *indi_generate_set_message(indi_dict_t *def_vector, STR_t set_tag_name, STR_t one_tag_name)
{
    /*----------------------------------------------------------------------------------------------------------------*/

    indi_dict_t *result = indi_dict_new();

    indi_list_t *children = indi_list_new();

    /*----------------------------------------------------------------------------------------------------------------*/

    indi_dict_set(result, "<>", indi_string_from(set_tag_name));

    internal_copy_entry(result, def_vector, "@device");
    internal_copy_entry(result, def_vector, "@name");
    internal_copy_entry(result, def_vector, "@state");
    internal_copy_entry(result, def_vector, "@timeout");
    internal_copy_entry(result, def_vector, "@timestamp");
    internal_copy_entry(result, def_vector, "@message");

    indi_dict_set(result, "children", children);

    /*----------------------------------------------------------------------------------------------------------------*/

    int idx;

    indi_object_t *obj;

    indi_object_t *list = indi_dict_get(def_vector, "children");

    for(indi_list_iter_t iter = INDI_LIST_ITER(list); indi_list_iterate(&iter, &idx, &obj);)
    {
        indi_dict_t *dict = indi_dict_new();

        /*------------------------------------------------------------------------------------------------------------*/

        indi_dict_set(dict, "<>", indi_string_from(one_tag_name));

        internal_copy_entry(dict, (indi_dict_t *) obj, "$");

        /*------------------------------------------------------------------------------------------------------------*/

        indi_list_push(children, dict);
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    return result;
}

/*--------------------------------------------------------------------------------------------------------------------*/
