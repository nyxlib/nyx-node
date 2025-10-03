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
    nyx_dict_t *dict = nyx_stream_set_vector_new((nyx_dict_t *) object);

    str_t json = nyx_dict_to_string(dict);
    printf("** \033[91mNOT REGISTERED\033[0m **\n%s\n", json);
    nyx_memory_free(json);

    nyx_dict_free(dict);
}

/*--------------------------------------------------------------------------------------------------------------------*/
/* DEF                                                                                                                */
/*--------------------------------------------------------------------------------------------------------------------*/

nyx_dict_t *nyx_stream_def_new(STR_t name, __NULLABLE__ STR_t label)
{
    if(label == NULL || label[0] == '\0')
    {
        label = name;
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    nyx_dict_t *result = nyx_dict_new();

    nyx_dict_set(result, "<>", nyx_string_from("defStream"));

    nyx_dict_set(result, "@name", nyx_string_from(name));
    nyx_dict_set(result, "@label", nyx_string_from(label));

    /*----------------------------------------------------------------------------------------------------------------*/

    return result;
}

/*--------------------------------------------------------------------------------------------------------------------*/
/* DEF VECTOR                                                                                                         */
/*--------------------------------------------------------------------------------------------------------------------*/

nyx_dict_t *nyx_stream_def_vector_new(
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

    nyx_dict_set(result, "<>", nyx_string_from("defStreamVector"));

    nyx_dict_set(result, "children", children);

    /*----------------------------------------------------------------------------------------------------------------*/

    nyx_dict_set(result, "@client", nyx_string_from("unknown"));
    nyx_dict_set(result, "@device", nyx_string_from(device));
    nyx_dict_set(result, "@name", nyx_string_from(name));

    nyx_dict_set(result, "@state", nyx_string_from(nyx_state_to_str(state)));

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

nyx_dict_t *nyx_stream_set_vector_new(const nyx_dict_t *vector)
{
    return internal_def_to_set(vector, "setStreamVector", "oneStream");
}

/*--------------------------------------------------------------------------------------------------------------------*/
/* PUBLISHER                                                                                                          */
/*--------------------------------------------------------------------------------------------------------------------*/

bool nyx_stream_pub(const nyx_dict_t *vector, size_t max_len, __ZEROABLE__ size_t n_fields, const str_t field_names[], const size_t field_sizes[], const buff_t field_buffs[])
{
    /*----------------------------------------------------------------------------------------------------------------*/
    /* CHECK IF STREAM IS ENABLED                                                                                     */
    /*----------------------------------------------------------------------------------------------------------------*/

    if((vector->base.flags & NYX_FLAGS_STREAM_MASK) == 0)
    {
        return false;
    }

    /*----------------------------------------------------------------------------------------------------------------*/
    /* RETRIEVE INFORMATION                                                                                           */
    /*----------------------------------------------------------------------------------------------------------------*/

    nyx_node_t *node = vector->base.node;

    STR_t device = nyx_dict_get_string(vector, "@device");
    STR_t stream = nyx_dict_get_string(vector,  "@name" );

    if(node == NULL || device == NULL || stream == NULL)
    {
        NYX_LOG_ERROR("Stream vector not properly initialized");

        return false;
    }

    /*----------------------------------------------------------------------------------------------------------------*/
    /* PUBLISH STREAM                                                                                                 */
    /*----------------------------------------------------------------------------------------------------------------*/

    nyx_redis_pub(
        node,
        device,
        stream,
        max_len,
        n_fields,
        field_names,
        field_sizes,
        field_buffs
    );

    /*----------------------------------------------------------------------------------------------------------------*/

    return true;
}

/*--------------------------------------------------------------------------------------------------------------------*/
