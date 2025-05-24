/* NyxNode
 * Author: Jérôme ODIER <jerome.odier@lpsc.in2p3.fr>
 * SPDX-License-Identifier: GPL-2.0-only (Mongoose backend) or GPL-3.0+
 */

/*--------------------------------------------------------------------------------------------------------------------*/

#include <stdio.h>

#include "../nyx_node_internal.h"

/*--------------------------------------------------------------------------------------------------------------------*/

static void debug_callback(nyx_object_t *object, __UNUSED__ bool modified)
{
    nyx_dict_t *dict = nyx_number_set_vector_new((nyx_dict_t *) object);

    str_t json = nyx_dict_to_string(dict);
    printf("** \033[91mNOT REGISTERED\033[0m **\n%s\n", json);
    nyx_memory_free(json);

    nyx_dict_free(dict);
}

/*--------------------------------------------------------------------------------------------------------------------*/
/* DEF STREAM                                                                                                         */
/*--------------------------------------------------------------------------------------------------------------------*/

nyx_dict_t *nyx_stream_def_new(
    STR_t device,
    STR_t name,
    nyx_state_t state,
    nyx_opts_t *opts
) {
    nyx_dict_t *result = nyx_dict_new();

    /*----------------------------------------------------------------------------------------------------------------*/

    nyx_dict_set(result, "<>", nyx_string_from("defStream"));

    nyx_dict_set(result, "@client", nyx_string_from("unknown"));
    nyx_dict_set(result, "@device", nyx_string_from(device));
    nyx_dict_set(result, "@name", nyx_string_from(name));

    nyx_dict_set(result, "@state", nyx_string_from(nyx_state_to_str(state)));

    /*----------------------------------------------------------------------------------------------------------------*/

    internal_set_opts(result, opts);

    /*----------------------------------------------------------------------------------------------------------------*/

    result->base.out_callback = debug_callback;

    return result;
}

/*--------------------------------------------------------------------------------------------------------------------*/
/* SET STREAM                                                                                                         */
/*--------------------------------------------------------------------------------------------------------------------*/

nyx_dict_t *nyx_stream_set_new(const nyx_dict_t *stream_def)
{
    return internal_def_to_set(stream_def, "setStream", "N/A");
}

/*--------------------------------------------------------------------------------------------------------------------*/
