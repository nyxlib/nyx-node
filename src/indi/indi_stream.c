/* NyxNode
 * Author: Jérôme ODIER <jerome.odier@lpsc.in2p3.fr>
 * SPDX-License-Identifier: GPL-2.0-only (Mongoose backend) or GPL-3.0+
 */

/*--------------------------------------------------------------------------------------------------------------------*/

#include "../nyx_node_internal.h"

/*--------------------------------------------------------------------------------------------------------------------*/

nyx_dict_t *nyx_stream_new(
    STR_t device,
    STR_t name,
    nyx_state_t state
) {
    nyx_dict_t *result = nyx_dict_new();

    /*----------------------------------------------------------------------------------------------------------------*/

    nyx_dict_set(result, "<>", nyx_string_from("stream"));

    nyx_dict_set(result, "@client", nyx_string_from("unknown"));
    nyx_dict_set(result, "@device", nyx_string_from(device));
    nyx_dict_set(result, "@name", nyx_string_from(name));

    nyx_dict_set(result, "@state", nyx_string_from(nyx_state_to_str(state)));

    /*----------------------------------------------------------------------------------------------------------------*/

    return result;
}

/*--------------------------------------------------------------------------------------------------------------------*/
