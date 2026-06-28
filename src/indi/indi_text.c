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

nyx_dict_t *nyx_text_prop_new(STR_t name, STR_t label, STR_t value)
{
    if(label == NULL || label[0] == '\0')
    {
        label = name;
    }

    if(value == NULL)
    {
        value = "";
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    nyx_dict_t *result = nyx_dict_new();

    nyx_dict_set_string_managed_unref(result, "<>", nyx_string_dup("defText"));

    nyx_dict_set_string_managed_unref(result, "@name", nyx_string_dup(name));
    nyx_dict_set_string_managed_unref(result, "@label", nyx_string_dup(label));

    /*----------------------------------------------------------------------------------------------------------------*/

    nyx_dict_set_string_managed_unref(result, "$", nyx_string_dup(value));

    /*----------------------------------------------------------------------------------------------------------------*/

    return result;
}

/*--------------------------------------------------------------------------------------------------------------------*/
/* PROP SETTER & GETTER                                                                                               */
/*--------------------------------------------------------------------------------------------------------------------*/

bool nyx_text_prop_set(nyx_dict_t *prop, STR_t value)
{
    if(value == NULL)
    {
        value = "";
    }

    return  nyx_dict_set_string_managed_unref(prop, "$", nyx_string_dup(value));
}

/*--------------------------------------------------------------------------------------------------------------------*/

STR_t nyx_text_prop_get(const nyx_dict_t *prop)
{
    return nyx_string_get((nyx_string_t *) nyx_dict_get(prop, "$"));
}

/*--------------------------------------------------------------------------------------------------------------------*/
/* VECTOR                                                                                                             */
/*--------------------------------------------------------------------------------------------------------------------*/

nyx_dict_t *nyx_text_vector_new(
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

    nyx_dict_set_string_managed_unref(result, "<>", nyx_string_dup("defTextVector"));

    /*----------------------------------------------------------------------------------------------------------------*/

    nyx_dict_set_string_managed_unref(result, "@client", nyx_string_dup("unknown"));
    nyx_dict_set_string_managed_unref(result, "@device", nyx_string_dup(device));
    nyx_dict_set_string_managed_unref(result, "@name", nyx_string_dup(name));

    nyx_dict_set_string_managed_unref(result, "@state", nyx_string_dup(nyx_state_to_str(state)));
    nyx_dict_set_string_managed_unref(result, "@perm", nyx_string_dup(nyx_perm_to_str(perm)));

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
/* SET VECTOR                                                                                                         */
/*--------------------------------------------------------------------------------------------------------------------*/

nyx_dict_t *nyx_text_set_vector_new(const nyx_dict_t *vector)
{
    return internal_prop_to_set_vector(vector, "setTextVector", "oneText");
}

/*--------------------------------------------------------------------------------------------------------------------*/
