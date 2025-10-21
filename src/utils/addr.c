/* NyxNode
 * Author: Jérôme ODIER <jerome.odier@lpsc.in2p3.fr>
 * SPDX-License-Identifier: GPL-2.0-only (Mongoose backend) or GPL-3.0+
 */

/*--------------------------------------------------------------------------------------------------------------------*/

#include <string.h>

#include "../nyx_node_internal.h"

/*--------------------------------------------------------------------------------------------------------------------*/

void nyx_generate_mac_addr(uint8_t result_mac[6], uint8_t mac0, uint8_t mac1, STR_t node_id)
{
    size_t size = node_id != NULL ? strlen(node_id)
                                  : 0x0000000000000
    ;

    uint32_t hash = nyx_hash32(size, node_id, 0xAABBCCDD);

    result_mac[0] = mac0;
    result_mac[1] = mac1;
    result_mac[2] = (uint8_t) ((hash >> 24) & 0xFF);
    result_mac[3] = (uint8_t) ((hash >> 16) & 0xFF);
    result_mac[4] = (uint8_t) ((hash >> 8) & 0xFF);
    result_mac[5] = (uint8_t) ((hash >> 0) & 0xFF);
}

/*--------------------------------------------------------------------------------------------------------------------*/
