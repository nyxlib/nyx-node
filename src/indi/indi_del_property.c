/* NyxNode
 * Author: Jérôme ODIER <jerome.odier@lpsc.in2p3.fr>
 * SPDX-License-Identifier: GPL-2.0-only (Mongoose backend) or GPL-3.0+
 */

/*--------------------------------------------------------------------------------------------------------------------*/

#include "../nyx_node_internal.h"

/*--------------------------------------------------------------------------------------------------------------------*/

nyx_dict_t *nyx_del_property_new(
    STR_t device,
    STR_t name,
    STR_t message
) {
    nyx_dict_t *result = nyx_dict_new();

    /*----------------------------------------------------------------------------------------------------------------*/

    char timestamp[32];

    internal_get_timestamp(sizeof(timestamp), timestamp);

    /*----------------------------------------------------------------------------------------------------------------*/

    nyx_dict_set_string_managed_unref(result, "<>", nyx_string_dup("delProperty"));

    nyx_dict_set_string_managed_unref(result, "@client", nyx_string_dup("unknown"));
    nyx_dict_set_string_managed_unref(result, "@device", nyx_string_dup(device));
    nyx_dict_set_string_managed_unref(result, "@timestamp", nyx_string_dup(timestamp));

    /*----------------------------------------------------------------------------------------------------------------*/

    if(name != NULL) {
        nyx_dict_set_string_managed_unref(result, "@name", nyx_string_dup(name));
    }

    if(message != NULL) {
        nyx_dict_set_string_managed_unref(result, "@message", nyx_string_dup(message));
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    return result;
}

/*--------------------------------------------------------------------------------------------------------------------*/
