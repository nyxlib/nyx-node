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

void nyx_redis_pub(nyx_node_t *node, STR_t stream, size_t max_len, __ZEROABLE__ size_t n_fields, ...)
{
    if(node == NULL
       ||
       stream == NULL
       ||
       n_fields == 0x0U
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

        va_list ap;

        va_start(ap, n_fields);

        /*------------------------------------------------------------------------------------------------------------*/

        internal_redis_pub(node, NYX_STR_S(header_buff, header_size));

        /*------------------------------------------------------------------------------------------------------------*/

        for(size_t i = 0; i < n_fields; i++)
        {
            /*--------------------------------------------------------------------------------------------------------*/

            STR_t field = va_arg(ap, STR_t);

            if(field == NULL)
            {
                field = "";
            }

            /*--------------------------------------------------------------------------------------------------------*/

            size_t size = va_arg(ap, size_t);
            BUFF_t buff = va_arg(ap, BUFF_t);

            if(size == 0x0U
               ||
               buff == NULL
            ) {
                size = 0U;
                buff = "";
            }

            /*--------------------------------------------------------------------------------------------------------*/

            char field_buff[256];

            size_t field_size = snprintf(
                /*--*/(field_buff),
                sizeof(field_buff),
                "$%zu\r\n%s\r\n"
                "$%zu\r\n",
                strlen(field),
                /*--*/(field),
                size
            );

            if(field_size > 0 && field_size < sizeof(field_buff))
            {
                internal_redis_pub(node, NYX_STR_S(field_buff, field_size));

                internal_redis_pub(node, NYX_STR_S(buff, size));

                internal_redis_pub(node, NYX_STR_S("\r\n", 2));
            }

            /*--------------------------------------------------------------------------------------------------------*/
        }

        /*------------------------------------------------------------------------------------------------------------*/

        va_end(ap);

        /*------------------------------------------------------------------------------------------------------------*/
    }

    /*----------------------------------------------------------------------------------------------------------------*/
}

/*--------------------------------------------------------------------------------------------------------------------*/
