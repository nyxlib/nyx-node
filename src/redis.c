/* NyxNode
 * Author: Jérôme ODIER <jerome.odier@lpsc.in2p3.fr>
 * SPDX-License-Identifier: GPL-2.0-only (Mongoose backend) or GPL-3.0+
 */

/*--------------------------------------------------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include "nyx_node_internal.h"

/*--------------------------------------------------------------------------------------------------------------------*/

void nyx_redis_auth(nyx_node_t *node, __NULLABLE__ STR_t password)
{
    if(password != NULL)
    {
        char buff[256];

        size_t size = snprintf(
            /*--*/(buff),
            sizeof(buff),
            "*2\r\n"
            "$4\r\nAUTH\r\n"
            "$%zu\r\n%s\r\n",
            strlen(password),
            /*--*/(password)
        );

        if(size > 0 && size < sizeof(buff))
        {
            internal_redis_pub(node, NYX_STR_S(buff, size));
        }
    }
}

/*--------------------------------------------------------------------------------------------------------------------*/

void nyx_redis_pub(nyx_node_t *node, STR_t stream, size_t max_len, __ZEROABLE__ size_t n_fields, STR_t *names, size_t *sizes, buff_t *buffs)
{
    if(node == NULL
       ||
       stream == NULL
       ||
       n_fields == 0x0U
       ||
       names == NULL || sizes == NULL || buffs == NULL
    ) {
        return;
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    char max_len_buff[32];

    snprintf(
        /*--*/(max_len_buff),
        sizeof(max_len_buff),
        "%zu",
        max_len
    );

    /*----------------------------------------------------------------------------------------------------------------*/

    char header_buff[256];

    size_t header_size = snprintf(
        /*--*/(header_buff),
        sizeof(header_buff),
        "*%zu\r\n"
        "$4\r\nXADD\r\n"
        "$%zu\r\n%s\r\n"
        "$6\r\nMAXLEN\r\n"
        "$1\r\n~\r\n"
        "$%zu\r\n%s\r\n"
        "$1\r\n*\r\n",
        6 + 2 * n_fields,
        strlen(stream),
        /*--*/(stream),
        strlen(max_len_buff),
        /*--*/(max_len_buff)
    );

    if(header_size > 0 && header_size < sizeof(header_buff))
    {
        /*------------------------------------------------------------------------------------------------------------*/

        internal_redis_pub(node, NYX_STR_S(header_buff, header_size));

        /*------------------------------------------------------------------------------------------------------------*/

        for(size_t i = 0; i < n_fields; i++)
        {
            /*--------------------------------------------------------------------------------------------------------*/

            STR_t field = names[i];

            if(field == NULL)
            {
                field = "";
            }

            /*--------------------------------------------------------------------------------------------------------*/

            size_t value_size = sizes[i];
            buff_t value_buff = buffs[i];

            if(value_size == 0x0U
               ||
               value_buff == NULL
            ) {
                value_size = 0U;
                value_buff = "";
            }

            /*--------------------------------------------------------------------------------------------------------*/

            bool is_raw;

            if(field[0] == '#')
            {
                value_buff = nyx_base64_encode(&value_size, value_size, value_buff);

                is_raw = false;
            }
            else
            {
                is_raw = true;
            }

            /*--------------------------------------------------------------------------------------------------------*/

            char header_buff[256];

            size_t header_size = snprintf(
                /*--*/(header_buff),
                sizeof(header_buff),
                "$%zu\r\n%s\r\n"
                "$%zu\r\n",
                strlen(field),
                /*--*/(field),
                value_size
            );

            if(header_size > 0 && header_size < sizeof(header_buff))
            {
                internal_redis_pub(node, NYX_STR_S(header_buff, header_size));

                internal_redis_pub(node, NYX_STR_S(value_buff, value_size));

                internal_redis_pub(node, NYX_STR_S("\r\n", 2));
            }

            /*--------------------------------------------------------------------------------------------------------*/

            if(!is_raw) nyx_memory_free(value_buff);

            /*--------------------------------------------------------------------------------------------------------*/
        }

        /*------------------------------------------------------------------------------------------------------------*/
    }

    /*----------------------------------------------------------------------------------------------------------------*/
}

/*--------------------------------------------------------------------------------------------------------------------*/
