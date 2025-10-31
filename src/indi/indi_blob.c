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

nyx_dict_t *nyx_blob_prop_new(STR_t name, __NYX_NULLABLE__ STR_t label, __NYX_NULLABLE__ STR_t format, __NYX_NULLABLE__ size_t size, __NYX_NULLABLE__ BUFF_t buff, bool managed)
{
    if(label == NULL || label[0] == '\0')
    {
        label = name;
    }

    if(format == NULL || format[0] == '\0')
    {
        format = "raw";
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    nyx_dict_t *result = nyx_dict_new();

    nyx_dict_set(result, "<>", nyx_string_from_dup("defBLOB"));

    nyx_dict_set(result, "@name", nyx_string_from_dup(name));
    nyx_dict_set(result, "@label", nyx_string_from_dup(label));
    nyx_dict_set(result, "@format", nyx_string_from_dup(format));

    if(managed) {
        nyx_blob_prop_set_managed(result, size, buff);
    }
    else {
        nyx_blob_prop_set_unmanaged(result, size, buff);
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    return result;
}

/*--------------------------------------------------------------------------------------------------------------------*/
/* PROP SETTER & GETTER                                                                                               */
/*--------------------------------------------------------------------------------------------------------------------*/

bool nyx_blob_prop_set_managed(nyx_dict_t *prop, __NYX_ZEROABLE__ size_t size, __NYX_NULLABLE__ BUFF_t buff)
{
    if(size == 0x00 || buff == NULL)
    {
        size = 0x00;
        buff = ("");
    }

    return nyx_dict_set(prop, "$", nyx_string_from_buff_managed(size, buff));
}

/*--------------------------------------------------------------------------------------------------------------------*/

bool nyx_blob_prop_set_unmanaged(nyx_dict_t *prop, __NYX_ZEROABLE__ size_t size, __NYX_NULLABLE__ BUFF_t buff)
{
    if(size == 0x00 || buff == NULL)
    {
        size = 0x00;
        buff = ("");
    }

    return nyx_dict_set(prop, "$", nyx_string_from_buff_unmanaged(size, buff));
}

/*--------------------------------------------------------------------------------------------------------------------*/

void nyx_blob_prop_get(const nyx_dict_t *prop, __NYX_NULLABLE__ size_t *size, __NYX_NULLABLE__ buff_t *buff)
{
    nyx_string_get_buff((nyx_string_t *) nyx_dict_get(prop, "$"), size, buff);
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
    __NYX_NULLABLE__ const nyx_opts_t *opts
) {
    /*----------------------------------------------------------------------------------------------------------------*/

    nyx_dict_t *result = nyx_dict_new();

    nyx_list_t *children = nyx_list_new();

    /*----------------------------------------------------------------------------------------------------------------*/

    nyx_dict_set(result, "<>", nyx_string_from_dup("defBLOBVector"));

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
/* SET VECTOR                                                                                                         */
/*--------------------------------------------------------------------------------------------------------------------*/

nyx_dict_t *nyx_blob_set_vector_new(const nyx_dict_t *vector)
{
    return internal_prop_to_set_vector(vector, "setBLOBVector", "oneBLOB");
}

/*--------------------------------------------------------------------------------------------------------------------*/
