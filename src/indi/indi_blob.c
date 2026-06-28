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

nyx_dict_t *nyx_blob_prop_new(STR_t name, STR_t label, STR_t format, size_t size, BUFF_t buff, bool managed)
{
    if(label == NULL || label[0] == '\0')
    {
        label = name;
    }

    if(format == NULL || format[0] == '\0')
    {
        format = "raw";
    }

    if(size == 0x00 || buff == NULL)
    {
        size = 0x00;
        buff = ("");

        managed = false;
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    nyx_dict_t *result = nyx_dict_new();

    /*----------------------------------------------------------------------------------------------------------------*/

    nyx_dict_set_string_unref(result, "<>", "defBLOB", false);

    /*----------------------------------------------------------------------------------------------------------------*/

    nyx_dict_set_string_unref(result, "@name", nyx_string_dup(name), true);
    nyx_dict_set_string_unref(result, "@label", nyx_string_dup(label), true);
    nyx_dict_set_string_unref(result, "@format", nyx_string_dup(format), true);

    /*----------------------------------------------------------------------------------------------------------------*/

    nyx_dict_set_buff_unref(result, "$", size, buff, managed);

    /*----------------------------------------------------------------------------------------------------------------*/

    return result;
}

/*--------------------------------------------------------------------------------------------------------------------*/
/* PROP SETTER & GETTER                                                                                               */
/*--------------------------------------------------------------------------------------------------------------------*/

bool nyx_blob_prop_set(const nyx_dict_t *prop, size_t size, BUFF_t buff, bool managed)
{
    if(size == 0x00 || buff == NULL)
    {
        size = 0x00;
        buff = ("");

        managed = false;
    }

    return nyx_dict_set_string_buff(prop, "$", size, buff, managed);
}

/*--------------------------------------------------------------------------------------------------------------------*/

void nyx_blob_prop_get(const nyx_dict_t *prop, size_t *size, buff_t *buff)
{
    return nyx_dict_get_buff(prop, "$", size, buff);
}

/*--------------------------------------------------------------------------------------------------------------------*/
/* VECTOR                                                                                                             */
/*--------------------------------------------------------------------------------------------------------------------*/

nyx_dict_t *nyx_blob_vector_new(
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

    nyx_dict_set_string_unref(result, "<>", "defBLOBVector", false);

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
/* SET VECTOR                                                                                                         */
/*--------------------------------------------------------------------------------------------------------------------*/

nyx_dict_t *nyx_blob_set_vector_new(const nyx_dict_t *vector)
{
    return internal_prop_to_set_vector(vector, "setBLOBVector", "oneBLOB");
}

/*--------------------------------------------------------------------------------------------------------------------*/
