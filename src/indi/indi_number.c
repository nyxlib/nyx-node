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

nyx_dict_t *nyx_number_prop_new(STR_t name, STR_t label, STR_t format, nyx_variant_t min, nyx_variant_t max, nyx_variant_t step, nyx_variant_t value)
{
    if(label == NULL || label[0] == '\0')
    {
        label = name;
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    nyx_dict_t *result = nyx_dict_new();

    nyx_dict_set(result, "<>", nyx_string_from_dup("defNumber"));

    nyx_dict_set(result, "@name", nyx_string_from_dup(name));
    nyx_dict_set(result, "@label", nyx_string_from_dup(label));
    nyx_dict_set(result, "@format", nyx_string_from_dup(format));

    nyx_dict_set(result, "@min", internal_variant_to_string(format, min));
    nyx_dict_set(result, "@max", internal_variant_to_string(format, max));
    nyx_dict_set(result, "@step", internal_variant_to_string(format, step));

    nyx_dict_set(result, "$", internal_variant_to_string(format, value));

    /*----------------------------------------------------------------------------------------------------------------*/

    return result;
}

/*--------------------------------------------------------------------------------------------------------------------*/
/* PROP SETTER & GETTER                                                                                               */
/*--------------------------------------------------------------------------------------------------------------------*/

bool nyx_number_prop_set(nyx_dict_t *prop, nyx_variant_t value)
{
    STR_t format = nyx_string_get((nyx_string_t *) nyx_dict_get(prop, "@format"));

    nyx_string_t *string = internal_variant_to_string(format, value);

    return nyx_dict_set(prop, "$", string);
}

/*--------------------------------------------------------------------------------------------------------------------*/

nyx_variant_t nyx_number_prop_get(const nyx_dict_t *prop)
{
    STR_t format = nyx_string_get((nyx_string_t *) nyx_dict_get(prop, "@format"));

    const nyx_string_t *string = (nyx_string_t *) nyx_dict_get(prop, "$");

    return internal_string_to_variant(format, string);
}

/*--------------------------------------------------------------------------------------------------------------------*/
/* VECTOR                                                                                                             */
/*--------------------------------------------------------------------------------------------------------------------*/

nyx_dict_t *nyx_number_vector_new(
    STR_t device,
    STR_t name,
    nyx_state_t state,
    nyx_perm_t perm,
    nyx_dict_t *props[],
    const nyx_opts_t *opts
) {
    /*----------------------------------------------------------------------------------------------------------------*/

    nyx_dict_t *result = nyx_dict_new();

    nyx_list_t *children = nyx_list_new();

    /*----------------------------------------------------------------------------------------------------------------*/

    nyx_dict_set(result, "<>", nyx_string_from_dup("defNumberVector"));

    nyx_dict_set(result, "children", children);

    /*----------------------------------------------------------------------------------------------------------------*/

    nyx_dict_set(result, "@client", nyx_string_from_dup("unknown"));
    nyx_dict_set(result, "@device", nyx_string_from_dup(device));
    nyx_dict_set(result, "@name", nyx_string_from_dup(name));

    nyx_dict_set(result, "@state", nyx_string_from_dup(nyx_state_to_str(state)));
    nyx_dict_set(result, "@perm", nyx_string_from_dup(nyx_perm_to_str(perm)));

    /*----------------------------------------------------------------------------------------------------------------*/

    internal_set_opts(result, opts);

    /*----------------------------------------------------------------------------------------------------------------*/

    for(; *props != NULL; props++) nyx_list_push(children, *props);

    /*----------------------------------------------------------------------------------------------------------------*/

    return result;
}

/*--------------------------------------------------------------------------------------------------------------------*/
/* VECTOR                                                                                                             */
/*--------------------------------------------------------------------------------------------------------------------*/

nyx_dict_t *nyx_number_set_vector_new(const nyx_dict_t *vector)
{
    return internal_prop_to_set_vector(vector, "setNumberVector", "oneNumber");
}

/*--------------------------------------------------------------------------------------------------------------------*/
