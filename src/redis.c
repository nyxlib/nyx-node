/* NyxNode
 * Author: Jérôme ODIER <jerome.odier@lpsc.in2p3.fr>
 * SPDX-License-Identifier: GPL-2.0-only (Mongoose backend) or GPL-3.0+
 */

/*--------------------------------------------------------------------------------------------------------------------*/

#include <stdio.h>
#include <string.h>

#include "nyx_node_internal.h"

/*--------------------------------------------------------------------------------------------------------------------*/
/* HELPERS                                                                                                            */
/*--------------------------------------------------------------------------------------------------------------------*/

static size_t intlen(size_t n)
{
    size_t len;

    for(len = 1; n >= 10; len++)
    {
        n /= 10;
    }

    return len;
}

/*--------------------------------------------------------------------------------------------------------------------*/
/* REDIS STREAM                                                                                                       */
/*--------------------------------------------------------------------------------------------------------------------*/

void nyx_redis_auth(nyx_node_t *node, __NULLABLE__ STR_t pwd_buff)
{
    if(pwd_buff != NULL && pwd_buff[0] != '\0')
    {
        /*------------------------------------------------------------------------------------------------------------*/

        char header_buff[128];

        /*------------------------------------------------------------------------------------------------------------*/

        size_t pwd_size = strlen(pwd_buff);

        size_t header_size = snprintf(
            /*--*/(header_buff),
            sizeof(header_buff),
            "*2\r\n"
            "$4\r\nAUTH\r\n"
            "$%zu\r\n",
            pwd_size
        );

        if(header_size > 0 && header_size < sizeof(header_buff))
        {
            internal_redis_pub(node, NYX_STR_S(header_buff, header_size));

            internal_redis_pub(node, NYX_STR_S(pwd_buff, pwd_size));

            internal_redis_pub(node, NYX_STR_S("\r\n", 2));
        }

        /*------------------------------------------------------------------------------------------------------------*/
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

    char header_buff[256];

    /*----------------------------------------------------------------------------------------------------------------*/

    size_t header_size = snprintf(
        /*--*/(header_buff),
        sizeof(header_buff),
        "*%zu\r\n"
        "$4\r\nXADD\r\n"
        "$%zu\r\n%s\r\n"
        "$6\r\nMAXLEN\r\n"
        "$1\r\n~\r\n"
        "$%zu\r\n%zu\r\n"
        "$1\r\n*\r\n",
        6 + 2 * n_fields,
        strlen(stream),
        /*--*/(stream),
        intlen(max_len),
        /*--*/(max_len)
    );

    if(header_size > 0 && header_size < sizeof(header_buff))
    {
        /*------------------------------------------------------------------------------------------------------------*/

        internal_redis_pub(node, NYX_STR_S(header_buff, header_size));

        /*------------------------------------------------------------------------------------------------------------*/

        for(size_t i = 0; i < n_fields; i++)
        {
            /*--------------------------------------------------------------------------------------------------------*/

            STR_t name = names[i];
            size_t size = sizes[i];
            BUFF_t buff = buffs[i];

            /*--------------------------------------------------------------------------------------------------------*/

            bool allocated;

            size_t value_size;
            buff_t value_buff;

            if(size != 0x0U
               &&
               buff != NULL
            ) {
                if(name[0] == '#')
                {
                    value_buff = nyx_base64_encode(&value_size, size, buff);

                    allocated = true;
                }
                else
                {
                    value_size = (size_t) size;
                    value_buff = (buff_t) buff;

                    allocated = false;
                }
            }
            else
            {
                value_size = (size_t) 0U;
                value_buff = (buff_t) "";

                allocated = false;
            }

            /*--------------------------------------------------------------------------------------------------------*/

            header_size = snprintf(
                /*--*/(header_buff),
                sizeof(header_buff),
                "$%zu\r\n%s\r\n"
                "$%zu\r\n",
                strlen(name),
                /*--*/(name),
                value_size
            );

            if(header_size > 0 && header_size < sizeof(header_buff))
            {
                internal_redis_pub(node, NYX_STR_S(header_buff, header_size));

                internal_redis_pub(node, NYX_STR_S(value_buff, value_size));

                internal_redis_pub(node, NYX_STR_S("\r\n", 2));
            }

            /*--------------------------------------------------------------------------------------------------------*/

            if(allocated) nyx_memory_free(value_buff);

            /*--------------------------------------------------------------------------------------------------------*/
        }

        /*------------------------------------------------------------------------------------------------------------*/
    }

    /*----------------------------------------------------------------------------------------------------------------*/
}

/*--------------------------------------------------------------------------------------------------------------------*/
