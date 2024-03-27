/*--------------------------------------------------------------------------------------------------------------------*/

#include "indi_base_internal.h"

/*--------------------------------------------------------------------------------------------------------------------*/
/* STATE                                                                                                              */
/*--------------------------------------------------------------------------------------------------------------------*/

__NULLABLE__
STR_t indi_state_to_str(__NULLABLE__ indi_state_t state)
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

    return NULL;
}

/*--------------------------------------------------------------------------------------------------------------------*/
/* PERM                                                                                                               */
/*--------------------------------------------------------------------------------------------------------------------*/

__NULLABLE__
STR_t indi_perm_to_str(__NULLABLE__ indi_perm_t perm)
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

    return NULL;
}

/*--------------------------------------------------------------------------------------------------------------------*/
/* RULE                                                                                                               */
/*--------------------------------------------------------------------------------------------------------------------*/

__NULLABLE__
STR_t indi_rule_to_str(__NULLABLE__ indi_rule_t rule)
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

    return NULL;
}

/*--------------------------------------------------------------------------------------------------------------------*/
/* ONOFF                                                                                                              */
/*--------------------------------------------------------------------------------------------------------------------*/

__NULLABLE__
STR_t indi_onoff_to_str(__NULLABLE__ indi_onoff_t onoff)
{
    switch(onoff)
    {
        case INDI_ONOFF_ON:
            return "On";
        case INDI_ONOFF_OFF:
            return "Off";
    }

    return NULL;
}

/*--------------------------------------------------------------------------------------------------------------------*/

static void indi_apply_opts(indi_dict_t *dict, indi_opt_t *opt)
{
    if(opt != NULL)
    {
        if(opt->label != NULL) {
            indi_dict_set(dict, "@label", indi_string_from(opt->label));
        }

        if(opt->group != NULL) {
            indi_dict_set(dict, "@group", indi_string_from(opt->group));
        }

        if(opt->timeout > 0x00) {
            indi_dict_set(dict, "@timeout", indi_number_from(opt->timeout));
        }

        if(opt->timestamp != NULL) {
            indi_dict_set(dict, "@timestamp", indi_string_from(opt->timestamp));
        }

        if(opt->message != NULL) {
            indi_dict_set(dict, "@message", indi_string_from(opt->message));
        }
    }
}

/*--------------------------------------------------------------------------------------------------------------------*/
/* NUMBER VECTORS                                                                                                     */
/*--------------------------------------------------------------------------------------------------------------------*/

indi_dict_t *indi_number_vector_new(STR_t device, STR_t name, indi_perm_t perm, indi_state_t state, size_t n_defs, indi_number_def_t defs[], indi_opt_t *opt)
{
    /*----------------------------------------------------------------------------------------------------------------*/

    indi_dict_t *result = indi_dict_new();

    indi_list_t *children = indi_list_new();

    /*----------------------------------------------------------------------------------------------------------------*/

    indi_dict_set(result, "<>", indi_string_from("defNumberVector"));

    indi_dict_set(result, "children", children);

    /*----------------------------------------------------------------------------------------------------------------*/

    indi_dict_set(result, "@device", indi_string_from(device));
    indi_dict_set(result, "@name", indi_string_from(name));

    indi_dict_set(result, "@state", indi_string_from(indi_state_to_str(state)));
    indi_dict_set(result, "@perm", indi_string_from(indi_perm_to_str(perm)));

    /*----------------------------------------------------------------------------------------------------------------*/

    indi_apply_opts(result, opt);

    /*----------------------------------------------------------------------------------------------------------------*/

    for(int i = 0; i < n_defs; i++)
    {
        /*------------------------------------------------------------------------------------------------------------*/

        indi_number_def_t *def = &defs[i];

        /*------------------------------------------------------------------------------------------------------------*/

        if(def->label == NULL)
        {
            def->label = def->name;
        }

        /*------------------------------------------------------------------------------------------------------------*/

        indi_dict_t *dict = indi_dict_new();

        indi_dict_set(dict, "<>", indi_string_from("defNumber"));

        indi_dict_set(dict, "@name", indi_string_from(def->name));
        indi_dict_set(dict, "@label", indi_string_from(def->label));
        indi_dict_set(dict, "@format", indi_string_from(def->format));

        indi_dict_set(dict, "@min", indi_number_from(def->min));
        indi_dict_set(dict, "@max", indi_number_from(def->max));
        indi_dict_set(dict, "@step", indi_number_from(def->step));

        indi_dict_set(dict, "$", indi_number_from(def->value));

        /*------------------------------------------------------------------------------------------------------------*/

        indi_list_push(children, dict);

        /*------------------------------------------------------------------------------------------------------------*/
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    return result;
}

/*--------------------------------------------------------------------------------------------------------------------*/
/* TEXT VECTORS                                                                                                       */
/*--------------------------------------------------------------------------------------------------------------------*/

indi_dict_t *indi_text_vector_new(STR_t device, STR_t name, indi_perm_t perm, indi_state_t state, size_t n_defs, indi_text_def_t defs[], indi_opt_t *opt)
{
    /*----------------------------------------------------------------------------------------------------------------*/

    indi_dict_t *result = indi_dict_new();

    indi_list_t *children = indi_list_new();

    /*----------------------------------------------------------------------------------------------------------------*/

    indi_dict_set(result, "<>", indi_string_from("defTextVector"));

    indi_dict_set(result, "children", children);

    /*----------------------------------------------------------------------------------------------------------------*/

    indi_dict_set(result, "@device", indi_string_from(device));
    indi_dict_set(result, "@name", indi_string_from(name));

    indi_dict_set(result, "@state", indi_string_from(indi_state_to_str(state)));
    indi_dict_set(result, "@perm", indi_string_from(indi_perm_to_str(perm)));

    /*----------------------------------------------------------------------------------------------------------------*/

    indi_apply_opts(result, opt);

    /*----------------------------------------------------------------------------------------------------------------*/

    for(int i = 0; i < n_defs; i++)
    {
        /*------------------------------------------------------------------------------------------------------------*/

        indi_text_def_t *def = &defs[i];

        /*------------------------------------------------------------------------------------------------------------*/

        if(def->label == NULL)
        {
            def->label = def->name;
        }

        /*------------------------------------------------------------------------------------------------------------*/

        indi_dict_t *dict = indi_dict_new();

        indi_dict_set(dict, "<>", indi_string_from("defText"));

        indi_dict_set(dict, "@name", indi_string_from(def->name));
        indi_dict_set(dict, "@label", indi_string_from(def->label));

        indi_dict_set(dict, "$", indi_string_from(def->value));

        /*------------------------------------------------------------------------------------------------------------*/

        indi_list_push(children, dict);

        /*------------------------------------------------------------------------------------------------------------*/
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    return result;
}

/*--------------------------------------------------------------------------------------------------------------------*/
/* LIGHT VECTORS                                                                                                      */
/*--------------------------------------------------------------------------------------------------------------------*/

indi_dict_t *indi_light_vector_new(STR_t device, STR_t name, indi_state_t state, size_t n_defs, indi_light_def_t defs[], indi_opt_t *opt)
{
    /*----------------------------------------------------------------------------------------------------------------*/

    indi_dict_t *result = indi_dict_new();

    indi_list_t *children = indi_list_new();

    /*----------------------------------------------------------------------------------------------------------------*/

    indi_dict_set(result, "<>", indi_string_from("defLightVector"));

    indi_dict_set(result, "children", children);

    /*----------------------------------------------------------------------------------------------------------------*/

    indi_dict_set(result, "@device", indi_string_from(device));
    indi_dict_set(result, "@name", indi_string_from(name));

    indi_dict_set(result, "@state", indi_string_from(indi_state_to_str(state)));

    /*----------------------------------------------------------------------------------------------------------------*/

    indi_apply_opts(result, opt);

    /*----------------------------------------------------------------------------------------------------------------*/

    for(int i = 0; i < n_defs; i++)
    {
        /*------------------------------------------------------------------------------------------------------------*/

        indi_light_def_t *def = &defs[i];

        /*------------------------------------------------------------------------------------------------------------*/

        if(def->label == NULL)
        {
            def->label = def->name;
        }

        /*------------------------------------------------------------------------------------------------------------*/

        indi_dict_t *dict = indi_dict_new();

        indi_dict_set(dict, "<>", indi_string_from("defLight"));

        indi_dict_set(dict, "@name", indi_string_from(def->name));
        indi_dict_set(dict, "@label", indi_string_from(def->label));

        indi_dict_set(dict, "$", indi_string_from(indi_state_to_str(def->value)));

        /*------------------------------------------------------------------------------------------------------------*/

        indi_list_push(children, dict);

        /*------------------------------------------------------------------------------------------------------------*/
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    return result;
}

/*--------------------------------------------------------------------------------------------------------------------*/
/* SWITCH VECTORS                                                                                                     */
/*--------------------------------------------------------------------------------------------------------------------*/

indi_dict_t *indi_switch_vector_new(STR_t device, STR_t name, indi_state_t state, indi_perm_t perm, indi_rule_t rule, size_t n_defs, indi_switch_def_t defs[], indi_opt_t *opt)
{
    /*----------------------------------------------------------------------------------------------------------------*/

    indi_dict_t *result = indi_dict_new();

    indi_list_t *children = indi_list_new();

    /*----------------------------------------------------------------------------------------------------------------*/

    indi_dict_set(result, "<>", indi_string_from("defSwitchVector"));

    indi_dict_set(result, "children", children);

    /*----------------------------------------------------------------------------------------------------------------*/

    indi_dict_set(result, "@device", indi_string_from(device));
    indi_dict_set(result, "@name", indi_string_from(name));

    indi_dict_set(result, "@state", indi_string_from(indi_state_to_str(state)));
    indi_dict_set(result, "@perm", indi_string_from(indi_perm_to_str(perm)));
    indi_dict_set(result, "@rule", indi_string_from(indi_rule_to_str(rule)));

    /*----------------------------------------------------------------------------------------------------------------*/

    indi_apply_opts(result, opt);

    /*----------------------------------------------------------------------------------------------------------------*/

    for(int i = 0; i < n_defs; i++)
    {
        /*------------------------------------------------------------------------------------------------------------*/

        indi_switch_def_t *def = &defs[i];

        /*------------------------------------------------------------------------------------------------------------*/

        if(def->label == NULL)
        {
            def->label = def->name;
        }

        /*------------------------------------------------------------------------------------------------------------*/

        indi_dict_t *dict = indi_dict_new();

        indi_dict_set(dict, "<>", indi_string_from("defSwitch"));

        indi_dict_set(dict, "@name", indi_string_from(def->name));
        indi_dict_set(dict, "@label", indi_string_from(def->label));

        indi_dict_set(dict, "$", indi_string_from(indi_onoff_to_str(def->value)));

        /*------------------------------------------------------------------------------------------------------------*/

        indi_list_push(children, dict);

        /*------------------------------------------------------------------------------------------------------------*/
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    return result;
}

/*--------------------------------------------------------------------------------------------------------------------*/
