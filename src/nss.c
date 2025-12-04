/* NyxNode
* Author: Jérôme ODIER <jerome.odier@lpsc.in2p3.fr>
 * SPDX-License-Identifier: GPL-2.0-only (Mongoose backend) or GPL-3.0+
 */

/*--------------------------------------------------------------------------------------------------------------------*/

#include <stdio.h>
#include <string.h>

#include "nyx_node_internal.h"

/*--------------------------------------------------------------------------------------------------------------------*/

#define STREAM_MAGIC 0x5358594EU

/*--------------------------------------------------------------------------------------------------------------------*/

void nyx_nss_pub(nyx_node_t *node, STR_t device, STR_t stream, size_t n_fields, const uint32_t field_hashes[], const size_t field_sizes[], const buff_t field_buffs[])
{
    /*----------------------------------------------------------------------------------------------------------------*/

    size_t path_size = strlen(device)
                       + 1 +
                       strlen(stream)
    ;

    char path_buff[path_size + 1];

    snprintf(path_buff, path_size + 1, "%s/%s", device, stream);

    uint32_t hash = nyx_hash(path_size, path_buff, STREAM_MAGIC);

    /*----------------------------------------------------------------------------------------------------------------*/

    uint32_t size = 0;

    for(size_t i = 0; i < n_fields; i++)
    {
        size += 8U + (uint32_t) field_sizes[i];
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    uint32_t header1[3] = {
        STREAM_MAGIC,
        hash,
        size,
    };

    internal_stream_pub(node, NYX_STR_S(buffof(header1), sizeof(header1)));

    /*----------------------------------------------------------------------------------------------------------------*/

    for(size_t i = 0; i < n_fields; i++)
    {
        uint32_t header2[2] = {
            field_hashes[i] & 0xFFFFFFFF,
            field_sizes[i] & 0xFFFFFFFF,
        };

        internal_stream_pub(node, NYX_STR_S(buffof(header2), sizeof(header2)));

        internal_stream_pub(node, NYX_STR_S(field_buffs[i], field_sizes[i]));
    }

    /*----------------------------------------------------------------------------------------------------------------*/
}

/*--------------------------------------------------------------------------------------------------------------------*/
