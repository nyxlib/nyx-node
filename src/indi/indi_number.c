/*--------------------------------------------------------------------------------------------------------------------*/

#include <stdio.h>

#include "../indi_base_internal.h"

/*--------------------------------------------------------------------------------------------------------------------*/

static void debug_callback(indi_object_t *object)
{
    indi_dict_t *dict = indi_number_set_vector_new((indi_dict_t *) object);

    str_t json = indi_dict_to_string(dict);
    printf("** \033[91mNOT REGISTERED\033[0m **\n%s\n", json);
    indi_memory_free(json);

    indi_dict_free(dict);
}

/*--------------------------------------------------------------------------------------------------------------------*/
/* DEF                                                                                                                */
/*--------------------------------------------------------------------------------------------------------------------*/

indi_dict_t *indi_number_def_new(STR_t name, __NULLABLE__ STR_t label, STR_t format, float min, float max, float step, float value)
{
    if(label == NULL)
    {
        label = name;
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    indi_dict_t *result = indi_dict_new();

    indi_dict_set(result, "<>", indi_string_from("defNumber"));

    indi_dict_set(result, "@name", indi_string_from(name));
    indi_dict_set(result, "@label", indi_string_from(label));
    indi_dict_set(result, "@format", indi_string_from(format));

    indi_dict_set(result, "@min", indi_number_from(min));
    indi_dict_set(result, "@max", indi_number_from(max));
    indi_dict_set(result, "@step", indi_number_from(step));

    indi_dict_set(result, "$", indi_number_from(value));

    /*----------------------------------------------------------------------------------------------------------------*/

    return result;
}

/*--------------------------------------------------------------------------------------------------------------------*/
/* DEF VECTOR                                                                                                         */
/*--------------------------------------------------------------------------------------------------------------------*/

indi_dict_t *indi_number_def_vector_new(
    STR_t device,
    STR_t name,
    indi_perm_t perm,
    indi_state_t state,
    indi_dict_t *defs[],
    indi_opt_t *opt
) {
    /*----------------------------------------------------------------------------------------------------------------*/

    indi_dict_t *result = indi_dict_new();

    indi_list_t *children = indi_list_new();

    /*----------------------------------------------------------------------------------------------------------------*/

    indi_dict_set(result, "<>", indi_string_from("defNumberVector"));

    indi_dict_set(result, "children", children);

    /*----------------------------------------------------------------------------------------------------------------*/

    indi_dict_set(result, "@client", indi_string_from("unknown"));
    indi_dict_set(result, "@device", indi_string_from(device));
    indi_dict_set(result, "@name", indi_string_from(name));

    indi_dict_set(result, "@state", indi_string_from(indi_state_to_str(state)));
    indi_dict_set(result, "@perm", indi_string_from(indi_perm_to_str(perm)));

    /*----------------------------------------------------------------------------------------------------------------*/

    internal_set_opts(result, opt);

    /*----------------------------------------------------------------------------------------------------------------*/

    for(; *defs != NULL; defs++) indi_list_push(children, *defs);

    /*----------------------------------------------------------------------------------------------------------------*/

    result->base.out_callback = debug_callback;

    return result;
}

/*--------------------------------------------------------------------------------------------------------------------*/
/* SET VECTOR                                                                                                         */
/*--------------------------------------------------------------------------------------------------------------------*/

indi_dict_t *indi_number_set_vector_new(const indi_dict_t *def_vector)
{
    return internal_xxx_set_vector_new(def_vector, "setNumberVector", "oneNumber");
}

/*--------------------------------------------------------------------------------------------------------------------*/
