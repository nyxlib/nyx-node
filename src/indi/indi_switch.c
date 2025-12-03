/* NyxNode
 * Author: Jérôme ODIER <jerome.odier@lpsc.in2p3.fr>
 * SPDX-License-Identifier: GPL-2.0-only (Mongoose backend) or GPL-3.0+
 */

/*--------------------------------------------------------------------------------------------------------------------*/

#include <stdio.h>

#include "../nyx_node_internal.h"

/*--------------------------------------------------------------------------------------------------------------------*/
/* PROP                                                                                                               */
/*--------------------------------------------------------------------------------------------------------------------*/

nyx_dict_t *nyx_switch_prop_new(STR_t name, STR_t label, nyx_onoff_t value)
{
    if(label == NULL || label[0] == '\0')
    {
        label = name;
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    nyx_dict_t *result = nyx_dict_new();

    nyx_dict_set(result, "<>", nyx_string_from_dup("defSwitch"));

    nyx_dict_set(result, "@name", nyx_string_from_dup(name));
    nyx_dict_set(result, "@label", nyx_string_from_dup(label));

    nyx_dict_set(result, "$", nyx_string_from_dup(nyx_onoff_to_str(value)));

    /*----------------------------------------------------------------------------------------------------------------*/

    return result;
}

/*--------------------------------------------------------------------------------------------------------------------*/
/* PROP SETTER & GETTER                                                                                               */
/*--------------------------------------------------------------------------------------------------------------------*/

bool nyx_switch_prop_set(nyx_dict_t *prop, nyx_onoff_t value)
{
    return nyx_dict_set(prop, "$", nyx_string_from_dup(nyx_onoff_to_str(value)));
}

/*--------------------------------------------------------------------------------------------------------------------*/

nyx_onoff_t nyx_switch_prop_get(const nyx_dict_t *prop)
{
    return nyx_str_to_onoff(nyx_string_get((nyx_string_t *) nyx_dict_get(prop, "$")));
}

/*--------------------------------------------------------------------------------------------------------------------*/
/* VECTOR                                                                                                             */
/*--------------------------------------------------------------------------------------------------------------------*/

nyx_dict_t *nyx_switch_vector_new(
    STR_t device,
    STR_t name,
    nyx_state_t state,
    nyx_perm_t perm,
    nyx_rule_t rule,
    nyx_dict_t *props[],
    const nyx_opts_t *opts
) {
    /*----------------------------------------------------------------------------------------------------------------*/

    nyx_dict_t *result = nyx_dict_new();

    nyx_list_t *children = nyx_list_new();

    /*----------------------------------------------------------------------------------------------------------------*/

    nyx_dict_set(result, "<>", nyx_string_from_dup("defSwitchVector"));

    nyx_dict_set(result, "children", children);

    /*----------------------------------------------------------------------------------------------------------------*/

    nyx_dict_set(result, "@client", nyx_string_from_dup("unknown"));
    nyx_dict_set(result, "@device", nyx_string_from_dup(device));
    nyx_dict_set(result, "@name", nyx_string_from_dup(name));

    nyx_dict_set(result, "@state", nyx_string_from_dup(nyx_state_to_str(state)));
    nyx_dict_set(result, "@perm", nyx_string_from_dup(nyx_perm_to_str(perm)));
    nyx_dict_set(result, "@rule", nyx_string_from_dup(nyx_rule_to_str(rule)));

    /*----------------------------------------------------------------------------------------------------------------*/

    internal_set_opts(result, opts);

    /*----------------------------------------------------------------------------------------------------------------*/

    for(; *props != NULL; props++) nyx_list_push(children, *props);

    /*----------------------------------------------------------------------------------------------------------------*/

    return result;
}

/*--------------------------------------------------------------------------------------------------------------------*/
/* SET VECTOR                                                                                                         */
/*--------------------------------------------------------------------------------------------------------------------*/

nyx_dict_t *nyx_switch_set_vector_new(const nyx_dict_t *vector)
{
    return internal_prop_to_set_vector(vector, "setSwitchVector", "oneSwitch");
}

/*--------------------------------------------------------------------------------------------------------------------*/
