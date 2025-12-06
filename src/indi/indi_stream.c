/* NyxNode
 * Author: Jérôme ODIER <jerome.odier@lpsc.in2p3.fr>
 * SPDX-License-Identifier: GPL-2.0-only (Mongoose backend) or GPL-3.0+
 */

/*--------------------------------------------------------------------------------------------------------------------*/

#include <stdio.h>
#include <string.h>

#include "../nyx_node_internal.h"

/*--------------------------------------------------------------------------------------------------------------------*/
/* PROP                                                                                                               */
/*--------------------------------------------------------------------------------------------------------------------*/

nyx_dict_t *nyx_stream_prop_new(STR_t name, STR_t label)
{
    if(label == NULL || label[0] == '\0')
    {
        label = name;
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    char hash[8 + 1];

    snprintf(hash, sizeof(hash), "%08X", nyx_hash(
        strlen(name),
        buffof(name),
        NYX_STREAM_MAGIC
    ));

    /*----------------------------------------------------------------------------------------------------------------*/

    nyx_dict_t *result = nyx_dict_new();

    nyx_dict_set(result, "<>", nyx_string_from_dup("defStream"));

    nyx_dict_set(result, "@name", nyx_string_from_dup(name));
    nyx_dict_set(result, "@hash", nyx_string_from_dup(hash));
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
    const nyx_opts_t *opts
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

bool nyx_stream_pub(const nyx_dict_t *vector, size_t n_fields, const size_t field_sizes[], const buff_t field_buffs[])
{
    /*----------------------------------------------------------------------------------------------------------------*/
    /* CHECK IF STREAM IS ENABLED OR EMPTY                                                                            */
    /*----------------------------------------------------------------------------------------------------------------*/

    if((vector->base.flags & NYX_FLAGS_STREAM_MASK) == 0 || n_fields == 0)
    {
        return true;
    }

    /*----------------------------------------------------------------------------------------------------------------*/
    /* RETRIEVE INFORMATION                                                                                           */
    /*----------------------------------------------------------------------------------------------------------------*/

    const nyx_node_t *node = vector->base.node;

    STR_t device = nyx_dict_get_string(vector, "@device");
    STR_t stream = nyx_dict_get_string(vector,  "@name" );

    if(node == NULL || device == NULL || stream == NULL)
    {
        NYX_LOG_ERROR("Stream vector not properly initialized");

        return false;
    }

    /*----------------------------------------------------------------------------------------------------------------*/
    /* PREPROCESS DATA                                                                                                */
    /*----------------------------------------------------------------------------------------------------------------*/

    size_t idx = 0;

    nyx_object_t *dict;

    uint32_t prepd_hashes[n_fields];

    size_t prepd_sizes[n_fields];
    BUFF_t prepd_buffs[n_fields];

    /*----------------------------------------------------------------------------------------------------------------*/

    nyx_object_t *list = nyx_dict_get(vector, "children");

    if(list != NULL && list->type == NYX_TYPE_LIST)
    {
        /*------------------------------------------------------------------------------------------------------------*/

        for(nyx_list_iter_t iter = NYX_LIST_ITER(list); nyx_list_iterate(&iter, &idx, &dict);)
        {
            if(dict->type == NYX_TYPE_DICT && idx < n_fields)
            {
                nyx_object_t *string = nyx_dict_get((nyx_dict_t *) dict, "@name");

                if(string != NULL && string->type == NYX_TYPE_STRING)
                {
                    /*------------------------------------------------------------------------------------------------*/

                    STR_t field_name_buf = nyx_string_get((nyx_string_t *) string);

                    size_t field_name_len = strlen(field_name_buf);

                    size_t field_size = field_sizes[idx];
                    BUFF_t field_buff = field_buffs[idx];

                    /*------------------------------------------------------------------------------------------------*/
                    /* COMPUTE HASH                                                                                   */
                    /*------------------------------------------------------------------------------------------------*/

                    prepd_hashes[idx] = nyx_hash(field_name_len, field_name_buf, NYX_STREAM_MAGIC);

                    /*------------------------------------------------------------------------------------------------*/
                    /* BASE64 PAYLOAD                                                                                 */
                    /*------------------------------------------------------------------------------------------------*/

                    if(field_name_len > 2 && field_name_buf[field_name_len - 2] == '.' && field_name_buf[field_name_len - 1] == 'b')
                    {
                        prepd_buffs[idx] = nyx_base64_encode(&prepd_sizes[idx], field_size, field_buff);
                    }

                    /*------------------------------------------------------------------------------------------------*/
                    /* ZLIB PAYLOAD                                                                                   */
                    /*------------------------------------------------------------------------------------------------*/

                    if(field_name_len > 2 && field_name_buf[field_name_len - 2] == '.' && field_name_buf[field_name_len - 1] == 'z')
                    {
                        prepd_buffs[idx] = nyx_zlib_deflate(&prepd_sizes[idx], field_size, field_buff);
                    }

                    /*------------------------------------------------------------------------------------------------*/
                    /* RAW PAYLOAD                                                                                    */
                    /*------------------------------------------------------------------------------------------------*/

                    else
                    {
                        prepd_sizes[idx] = (size_t) /* NOSONAR */ field_size;
                        prepd_buffs[idx] = (buff_t) /* NOSONAR */ field_buff;
                    }

                    /*------------------------------------------------------------------------------------------------*/
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

    /*----------------------------------------------------------------------------------------------------------------*/

    if(n_fields != idx + 1)
    {
        NYX_LOG_ERROR("Missing fields, %d expected, %d provided", (int) n_fields, (int) idx + 1);

        return false;
    }

    /*----------------------------------------------------------------------------------------------------------------*/
    /* PUBLISH STREAM                                                                                                 */
    /*----------------------------------------------------------------------------------------------------------------*/

    nyx_nss_pub(node, device, stream, n_fields, prepd_hashes, prepd_sizes, prepd_buffs);

    /*----------------------------------------------------------------------------------------------------------------*/

    for(size_t i = 0; i < n_fields; i++)
    {
        if(prepd_buffs[i] != field_buffs[i])
        {
            nyx_memory_free((buff_t) /* NOSONAR */ prepd_buffs[i]);
        }
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    return true;
}

/*--------------------------------------------------------------------------------------------------------------------*/
