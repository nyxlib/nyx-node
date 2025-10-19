/* NyxNode
 * Author: Jérôme ODIER <jerome.odier@lpsc.in2p3.fr>
 * SPDX-License-Identifier: GPL-2.0-only (Mongoose backend) or GPL-3.0+
 */

/*--------------------------------------------------------------------------------------------------------------------*/

#include <stdio.h>

#include "../nyx_node_internal.h"

/*--------------------------------------------------------------------------------------------------------------------*/

static void _debug_callback(nyx_object_t *object)
{
    nyx_dict_t *dict = nyx_light_set_vector_new((nyx_dict_t *) object);

    str_t json = nyx_dict_to_string(dict);
    printf("** \033[91mNOT REGISTERED\033[0m **\n%s\n", json);
    nyx_memory_free(json);

    nyx_dict_free(dict);
}

/*--------------------------------------------------------------------------------------------------------------------*/
/* PROP                                                                                                               */
/*--------------------------------------------------------------------------------------------------------------------*/

nyx_dict_t *nyx_light_prop_new(STR_t name, __NULLABLE__ STR_t label, nyx_state_t value)
{
    if(label == NULL || label[0] == '\0')
    {
        label = name;
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    nyx_dict_t *result = nyx_dict_new();

    nyx_dict_set(result, "<>", nyx_string_from_dup("defLight"));

    nyx_dict_set(result, "@name", nyx_string_from_dup(name));
    nyx_dict_set(result, "@label", nyx_string_from_dup(label));

    nyx_dict_set(result, "$", nyx_string_from_dup(nyx_state_to_str(value)));

    /*----------------------------------------------------------------------------------------------------------------*/

    return result;
}

/*--------------------------------------------------------------------------------------------------------------------*/
/* PROP SETTER & GETTER                                                                                               */
/*--------------------------------------------------------------------------------------------------------------------*/

bool nyx_light_prop_set(nyx_dict_t *prop, nyx_state_t value)
{
    return nyx_dict_set(prop, "$", nyx_string_from_dup(nyx_state_to_str(value)));
}

/*--------------------------------------------------------------------------------------------------------------------*/

nyx_state_t nyx_light_prop_get(const nyx_dict_t *prop)
{
    return nyx_str_to_state(nyx_string_get((nyx_string_t *) nyx_dict_get(prop, "$")));
}

/*--------------------------------------------------------------------------------------------------------------------*/
/* VECTOR                                                                                                             */
/*--------------------------------------------------------------------------------------------------------------------*/

nyx_dict_t *nyx_light_vector_new(
    STR_t device,
    STR_t name,
    nyx_state_t state,
    nyx_dict_t *defs[],
    __NULLABLE__ const nyx_opts_t *opts
) {
    /*----------------------------------------------------------------------------------------------------------------*/

    nyx_dict_t *result = nyx_dict_new();

    nyx_list_t *children = nyx_list_new();

    /*----------------------------------------------------------------------------------------------------------------*/

    nyx_dict_set(result, "<>", nyx_string_from_dup("defLightVector"));

    nyx_dict_set(result, "children", children);

    /*----------------------------------------------------------------------------------------------------------------*/

    nyx_dict_set(result, "@client", nyx_string_from_dup("unknown"));
    nyx_dict_set(result, "@device", nyx_string_from_dup(device));
    nyx_dict_set(result, "@name", nyx_string_from_dup(name));

    nyx_dict_set(result, "@state", nyx_string_from_dup(nyx_state_to_str(state)));

    /*----------------------------------------------------------------------------------------------------------------*/

    internal_set_opts(result, opts);

    /*----------------------------------------------------------------------------------------------------------------*/

    for(; *defs != NULL; defs++) nyx_list_push(children, *defs);

    /*----------------------------------------------------------------------------------------------------------------*/

    result->base.out_callback = _debug_callback;

    return result;
}

/*--------------------------------------------------------------------------------------------------------------------*/
/* SET VECTOR                                                                                                         */
/*--------------------------------------------------------------------------------------------------------------------*/

nyx_dict_t *nyx_light_set_vector_new(const nyx_dict_t *vector)
{
    return internal_prop_to_set_vector(vector, "setLightVector", "oneLight");
}

/*--------------------------------------------------------------------------------------------------------------------*/
