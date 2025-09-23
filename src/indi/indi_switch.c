/* NyxNode
 * Author: Jérôme ODIER <jerome.odier@lpsc.in2p3.fr>
 * SPDX-License-Identifier: GPL-2.0-only (Mongoose backend) or GPL-3.0+
 */

/*--------------------------------------------------------------------------------------------------------------------*/

#include <stdio.h>

#include "../nyx_node_internal.h"

/*--------------------------------------------------------------------------------------------------------------------*/

static void debug_callback(nyx_object_t *object)
{
    nyx_dict_t *dict = nyx_switch_set_vector_new((nyx_dict_t *) object);

    str_t json = nyx_dict_to_string(dict);
    printf("** \033[91mNOT REGISTERED\033[0m **\n%s\n", json);
    nyx_memory_free(json);

    nyx_dict_free(dict);
}

/*--------------------------------------------------------------------------------------------------------------------*/
/* DEF                                                                                                                */
/*--------------------------------------------------------------------------------------------------------------------*/

nyx_dict_t *nyx_switch_def_new(STR_t name, __NULLABLE__ STR_t label, nyx_onoff_t value)
{
    if(label == NULL || label[0] == '\0')
    {
        label = name;
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    nyx_dict_t *result = nyx_dict_new();

    nyx_dict_set(result, "<>", nyx_string_from("defSwitch"));

    nyx_dict_set(result, "@name", nyx_string_from(name));
    nyx_dict_set(result, "@label", nyx_string_from(label));

    nyx_dict_set(result, "$", nyx_string_from(nyx_onoff_to_str(value)));

    /*----------------------------------------------------------------------------------------------------------------*/

    return result;
}

/*--------------------------------------------------------------------------------------------------------------------*/
/* DEF VECTOR                                                                                                         */
/*--------------------------------------------------------------------------------------------------------------------*/

nyx_dict_t *nyx_switch_def_vector_new(
    STR_t device,
    STR_t name,
    nyx_state_t state,
    nyx_perm_t perm,
    nyx_rule_t rule,
    nyx_dict_t *defs[],
    __NULLABLE__ const nyx_opts_t *opts
) {
    /*----------------------------------------------------------------------------------------------------------------*/

    nyx_dict_t *result = nyx_dict_new();

    nyx_list_t *children = nyx_list_new();

    /*----------------------------------------------------------------------------------------------------------------*/

    nyx_dict_set(result, "<>", nyx_string_from("defSwitchVector"));

    nyx_dict_set(result, "children", children);

    /*----------------------------------------------------------------------------------------------------------------*/

    nyx_dict_set(result, "@client", nyx_string_from("unknown"));
    nyx_dict_set(result, "@device", nyx_string_from(device));
    nyx_dict_set(result, "@name", nyx_string_from(name));

    nyx_dict_set(result, "@state", nyx_string_from(nyx_state_to_str(state)));
    nyx_dict_set(result, "@perm", nyx_string_from(nyx_perm_to_str(perm)));
    nyx_dict_set(result, "@rule", nyx_string_from(nyx_rule_to_str(rule)));

    /*----------------------------------------------------------------------------------------------------------------*/

    internal_set_opts(result, opts);

    /*----------------------------------------------------------------------------------------------------------------*/

    for(; *defs != NULL; defs++) nyx_list_push(children, *defs);

    /*----------------------------------------------------------------------------------------------------------------*/

    result->base.out_callback = debug_callback;

    return result;
}

/*--------------------------------------------------------------------------------------------------------------------*/
/* SET VECTOR                                                                                                         */
/*--------------------------------------------------------------------------------------------------------------------*/

nyx_dict_t *nyx_switch_set_vector_new(const nyx_dict_t *vector)
{
    return internal_def_to_set(vector, "setSwitchVector", "oneSwitch");
}

/*--------------------------------------------------------------------------------------------------------------------*/
/* SETTER & GETTER                                                                                                    */
/*--------------------------------------------------------------------------------------------------------------------*/

bool nyx_switch_def_set(nyx_dict_t *def, nyx_onoff_t value)
{
    return nyx_dict_set(def, "$", nyx_string_from(nyx_onoff_to_str(value)));
}

/*--------------------------------------------------------------------------------------------------------------------*/

nyx_onoff_t nyx_switch_def_get(const nyx_dict_t *def)
{
    return nyx_str_to_onoff(((nyx_string_t *) nyx_dict_get(def, "$"))->value);
}

/*--------------------------------------------------------------------------------------------------------------------*/
