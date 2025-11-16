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

nyx_dict_t *nyx_stream_prop_new(STR_t name, __NYX_NULLABLE__ STR_t label)
{
    if(label == NULL || label[0] == '\0')
    {
        label = name;
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    nyx_dict_t *result = nyx_dict_new();

    nyx_dict_set(result, "<>", nyx_string_from_dup("defStream"));

    nyx_dict_set(result, "@name", nyx_string_from_dup(name));
    nyx_dict_set(result, "@label", nyx_string_from_dup(label));

    /*----------------------------------------------------------------------------------------------------------------*/

    return result;
}

/*--------------------------------------------------------------------------------------------------------------------*/
/* VECTOR                                                                                                             */
/*--------------------------------------------------------------------------------------------------------------------*/

nyx_dict_t *nyx_stream_vector_new(
    STR_t device,
    STR_t name,
    nyx_state_t state,
    nyx_dict_t *props[],
    __NYX_NULLABLE__ const nyx_opts_t *opts
) {
    /*----------------------------------------------------------------------------------------------------------------*/

    nyx_dict_t *result = nyx_dict_new();

    nyx_list_t *children = nyx_list_new();

    /*----------------------------------------------------------------------------------------------------------------*/

    nyx_dict_set(result, "<>", nyx_string_from_dup("defStreamVector"));

    nyx_dict_set(result, "children", children);

    /*----------------------------------------------------------------------------------------------------------------*/

    nyx_dict_set(result, "@client", nyx_string_from_dup("unknown"));
    nyx_dict_set(result, "@device", nyx_string_from_dup(device));
    nyx_dict_set(result, "@name", nyx_string_from_dup(name));

    nyx_dict_set(result, "@state", nyx_string_from_dup(nyx_state_to_str(state)));

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

nyx_dict_t *nyx_stream_set_vector_new(const nyx_dict_t *vector)
{
    return internal_prop_to_set_vector(vector, "setStreamVector", "oneStream");
}

/*--------------------------------------------------------------------------------------------------------------------*/
/* PUBLISHER                                                                                                          */
/*--------------------------------------------------------------------------------------------------------------------*/

static bool _get_field_names(STR_t field_names[], int n_fields, const nyx_dict_t *vector)
{
    nyx_object_t *list = nyx_dict_get(vector, "children");

    if(list != NULL && list->type == NYX_TYPE_LIST)
    {
        /*------------------------------------------------------------------------------------------------------------*/

        int expected_n_fields = (int) nyx_list_size((nyx_list_t *) list);

        if(expected_n_fields != n_fields)
        {
            NYX_LOG_ERROR("%d expected fields but %d provided", expected_n_fields, n_fields);

            return false;
        }

        /*------------------------------------------------------------------------------------------------------------*/

        size_t idx;

        nyx_object_t *dict;

        for(nyx_list_iter_t iter = NYX_LIST_ITER(list); nyx_list_iterate(&iter, &idx, &dict);)
        {
            if(dict->type == NYX_TYPE_DICT)
            {
                nyx_object_t *string = nyx_dict_get((nyx_dict_t *) dict, "@name");

                if(string != NULL && string->type == NYX_TYPE_STRING)
                {
                    field_names[idx] = nyx_string_get((nyx_string_t *) string);
                }
                else
                {
                    NYX_LOG_ERROR("Invalid stream property");

                    return false;
                }
            }
            else
            {
                NYX_LOG_ERROR("Invalid stream property");

                return false;
            }
        }

        /*------------------------------------------------------------------------------------------------------------*/
    }
    else
    {
        NYX_LOG_ERROR("Invalid stream vector");

        return false;
    }

    return true;
}

/*--------------------------------------------------------------------------------------------------------------------*/

bool nyx_stream_pub(const nyx_dict_t *vector, size_t max_len, int n_fields, const size_t field_sizes[], const buff_t field_buffs[])
{
    /*----------------------------------------------------------------------------------------------------------------*/
    /* CHECK IF STREAM IS ENABLED                                                                                     */
    /*----------------------------------------------------------------------------------------------------------------*/

    if((vector->base.flags & NYX_FLAGS_STREAM_MASK) == 0)
    {
        return true;
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

    STR_t field_names[n_fields];

    if(!_get_field_names(field_names, n_fields, vector))
    {
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
