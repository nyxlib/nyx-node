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

__NYX_INLINE__ uint32_t _safe_strlen(STR_t s)
{
    return (uint32_t) strlen(s);
}

/*--------------------------------------------------------------------------------------------------------------------*/

__NYX_INLINE__ uint32_t _safe_intlen(size_t n)
{
    uint32_t len;

    for(len = 1; n >= 10; len++)
    {
        n /= 10;
    }

    return len;
}

/*--------------------------------------------------------------------------------------------------------------------*/
/* REDIS STREAM                                                                                                       */
/*--------------------------------------------------------------------------------------------------------------------*/

void nyx_redis_auth(nyx_node_t *node, __NYX_NULLABLE__ STR_t username_buff, __NYX_NULLABLE__ STR_t password_buff)
{
    if(node == NULL)
    {
        return;
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    if(password_buff != NULL && password_buff[0] != '\0')
    {
        uint32_t password_size = _safe_strlen(password_buff);

        if(username_buff != NULL && username_buff[0] != '\0')
        {
            uint32_t username_size = _safe_strlen(username_buff);

            /*--------------------------------------------------------------------------------------------------------*/

            char cmd_buff[64 + username_size + password_size];

            /*--------------------------------------------------------------------------------------------------------*/

            uint32_t cmd_size = (uint32_t) snprintf(
                /*--*/(cmd_buff),
                sizeof(cmd_buff),
                "*3\r\n"
                "$4\r\nAUTH\r\n"
                "$%u\r\n%s\r\n"
                "$%u\r\n%s\r\n",
                username_size,
                username_buff,
                password_size,
                password_buff
            );

            if(cmd_size > 0 && cmd_size < sizeof(cmd_buff))
            {
                internal_redis_pub(node, NYX_STR_S(cmd_buff, cmd_size));
            }

            /*--------------------------------------------------------------------------------------------------------*/
        }
        else
        {
            /*--------------------------------------------------------------------------------------------------------*/

            char cmd_buff[64 + password_size];

            /*--------------------------------------------------------------------------------------------------------*/

            uint32_t cmd_size = (uint32_t) snprintf(
                /*--*/(cmd_buff),
                sizeof(cmd_buff),
                "*2\r\n"
                "$4\r\nAUTH\r\n"
                "$%u\r\n%s\r\n",
                password_size,
                password_buff
            );

            if(cmd_size > 0 && cmd_size < sizeof(cmd_buff))
            {
                internal_redis_pub(node, NYX_STR_S(cmd_buff, cmd_size));
            }

            /*--------------------------------------------------------------------------------------------------------*/
        }
    }

    /*----------------------------------------------------------------------------------------------------------------*/
}

/*--------------------------------------------------------------------------------------------------------------------*/

void nyx_redis_pub(nyx_node_t *node, STR_t device, STR_t stream, size_t max_len, __NYX_ZEROABLE__ int n_fields, const str_t field_names[], const size_t field_sizes[], const buff_t field_buffs[])
{
    if(node == NULL
       ||
       device == NULL
       ||
       stream == NULL
       ||
       n_fields == 0x0U
       ||
       field_names == NULL || field_sizes == NULL || field_buffs == NULL
    ) {
        return;
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    char header_buff[256];

    /*----------------------------------------------------------------------------------------------------------------*/

    uint32_t header_size = (uint32_t) snprintf(
        /*--*/(header_buff),
        sizeof(header_buff),
        "*%d\r\n"
        "$4\r\nXADD\r\n"
        "$%u\r\n%s/%s\r\n"
        "$6\r\nMAXLEN\r\n"
        "$1\r\n~\r\n"
        "$%u\r\n%u\r\n"
        "$1\r\n*\r\n",
        6 + 2 * n_fields,
        _safe_strlen(device) + 1 + _safe_strlen(stream),
        /*--------*/(device)   ,   /*--------*/(stream),
        _safe_intlen(max_len),
        (uint32_t)(max_len)
    );

    if(header_size > 0 && header_size < sizeof(header_buff))
    {
        /*------------------------------------------------------------------------------------------------------------*/

        internal_redis_pub(node, NYX_STR_S(header_buff, header_size));

        /*------------------------------------------------------------------------------------------------------------*/

        for(int i = 0; i < n_fields; i++)
        {
            /*--------------------------------------------------------------------------------------------------------*/

            STR_t field_name = field_names[i];
            size_t field_size = field_sizes[i];
            BUFF_t field_buff = field_buffs[i];

            /*--------------------------------------------------------------------------------------------------------*/

            bool allocated;

            size_t value_size;
            buff_t value_buff;

            if(field_size != 0x0U
               &&
               field_buff != NULL
            ) {
                size_t len = strlen(field_name);

                if(len > 2 && field_name[len - 2] == '.' && field_name[len - 1] == 'z')
                {
                    value_buff = nyx_zlib_deflate(&value_size, field_size, field_buff);

                    allocated = true;
                }
                else
                {
                    value_size = (size_t) field_size;
                    value_buff = (buff_t) field_buff;

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

            header_size = (uint32_t) snprintf(
                /*--*/(header_buff),
                sizeof(header_buff),
                "$%u\r\n%s\r\n"
                "$%u\r\n",
                _safe_strlen(field_name),
                /*--------*/(field_name),
                (uint32_t)(value_size)
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
