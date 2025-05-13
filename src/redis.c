/* NyxNode
 * Author: Jérôme ODIER <jerome.odier@lpsc.in2p3.fr>
 * SPDX-License-Identifier: GPL-2.0-only (Mongoose backend) or GPL-3.0+
 */

/*--------------------------------------------------------------------------------------------------------------------*/

#include <stdio.h>
#include <string.h>

#include "nyx_node_internal.h"

/*--------------------------------------------------------------------------------------------------------------------*/

void nyx_redis_auth(nyx_node_t *node, __NULLABLE__ STR_t password)
{
    if(password != NULL && password[0] != '\0')
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

void nyx_redis_pub(nyx_node_t *node, STR_t stream, size_t max_len, __ZEROABLE__ size_t n_fields, const str_t names[], const size_t sizes[], const buff_t buffs[])
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

    char header_buff1[256];

    size_t header_size1 = snprintf(
        /*--*/(header_buff1),
        sizeof(header_buff1),
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

    if(header_size1 > 0 && header_size1 < sizeof(header_buff1))
    {
        /*------------------------------------------------------------------------------------------------------------*/

        internal_redis_pub(node, NYX_STR_S(header_buff1, header_size1));

        /*------------------------------------------------------------------------------------------------------------*/

        for(size_t i = 0; i < n_fields; i++)
        {
            /*--------------------------------------------------------------------------------------------------------*/

            STR_t name = names[i];
            size_t size = sizes[i];
            BUFF_t buff = buffs[i];

            /*--------------------------------------------------------------------------------------------------------*/

            bool q;

            size_t value_size;
            buff_t value_buff;

            if(size != 0x0U
               &&
               buff != NULL
            ) {
                if(name[0] == '#')
                {
                    value_buff = nyx_base64_encode(&value_size, size, buff);

                    q = true;
                }
                else
                {
                    value_size = (size_t) size;
                    value_buff = (buff_t) buff;

                    q = false;
                }
            }
            else
            {
                value_size = (size_t) 0U;
                value_buff = (buff_t) "";

                q = false;
            }

            /*--------------------------------------------------------------------------------------------------------*/

            char header_buff2[256];

            size_t header_size2 = snprintf(
                /*--*/(header_buff2),
                sizeof(header_buff2),
                "$%zu\r\n%s\r\n"
                "$%zu\r\n",
                strlen(name),
                /*--*/(name),
                value_size
            );

            if(header_size2 > 0 && header_size2 < sizeof(header_buff2))
            {
                internal_redis_pub(node, NYX_STR_S(header_buff2, header_size2));

                internal_redis_pub(node, NYX_STR_S(value_buff, value_size));

                internal_redis_pub(node, NYX_STR_S("\r\n", 2));
            }

            /*--------------------------------------------------------------------------------------------------------*/

            if(q) nyx_memory_free(value_buff);

            /*--------------------------------------------------------------------------------------------------------*/
        }

        /*------------------------------------------------------------------------------------------------------------*/
    }

    /*----------------------------------------------------------------------------------------------------------------*/
}

/*--------------------------------------------------------------------------------------------------------------------*/
