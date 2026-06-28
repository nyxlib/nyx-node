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

    /*----------------------------------------------------------------------------------------------------------------*/

    nyx_dict_set_string_unref(result, "<>", "defNumber", false);

    /*----------------------------------------------------------------------------------------------------------------*/

    nyx_dict_set_string_unref(result, "@name", nyx_string_dup(name), true);
    nyx_dict_set_string_unref(result, "@label", nyx_string_dup(label), true);
    nyx_dict_set_string_unref(result, "@format", nyx_string_dup(format), true);

    nyx_dict_set_string_unref(result, "@min", internal_variant_to_string(format, min), true);
    nyx_dict_set_string_unref(result, "@max", internal_variant_to_string(format, max), true);
    nyx_dict_set_string_unref(result, "@step", internal_variant_to_string(format, step), true);

    /*----------------------------------------------------------------------------------------------------------------*/

    nyx_dict_set_string_unref(result, "$", internal_variant_to_string(format, value), true);

    /*----------------------------------------------------------------------------------------------------------------*/

    return result;
}

/*--------------------------------------------------------------------------------------------------------------------*/
/* PROP SETTER & GETTER                                                                                               */
/*--------------------------------------------------------------------------------------------------------------------*/

bool nyx_number_prop_set(const nyx_dict_t *prop, nyx_variant_t value)
{
    STR_t format = nyx_dict_get_string(prop, "@format");

    return nyx_dict_set_string(prop, "$", internal_variant_to_string(format, value), true);
}

/*--------------------------------------------------------------------------------------------------------------------*/

nyx_variant_t nyx_number_prop_get(const nyx_dict_t *prop)
{
    STR_t format = nyx_dict_get_string(prop, "@format");

    return internal_string_to_variant(format, nyx_dict_get_string(prop, "$"));
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
    nyx_dict_t *result = nyx_dict_new();

    nyx_list_t *children = nyx_list_new();

    /*----------------------------------------------------------------------------------------------------------------*/

    nyx_dict_set_string_unref(result, "<>", "defNumberVector", false);

    /*----------------------------------------------------------------------------------------------------------------*/

    nyx_dict_set_string_unref(result, "@client", "unknown", false);
    nyx_dict_set_string_unref(result, "@device", nyx_string_dup(device), true);
    nyx_dict_set_string_unref(result, "@name", nyx_string_dup(name), true);

    nyx_dict_set_string_unref(result, "@state", nyx_state_to_str(state), false);
    nyx_dict_set_string_unref(result, "@perm", nyx_perm_to_str(perm), false);

    /*----------------------------------------------------------------------------------------------------------------*/

    internal_set_opts(result, opts);

    /*----------------------------------------------------------------------------------------------------------------*/

    nyx_dict_set(result, "children", children);

    if(props) for(; *props != NULL; props++)
    {
        nyx_list_push(children, *props);

        nyx_object_unref(*props);
    }

    nyx_object_unref(children);

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
