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

void nyx_redis_auth(nyx_node_t *node, __NULLABLE__ STR_t username_buff, __NULLABLE__ STR_t password_buff)
{
    if(password_buff != NULL && password_buff[0] != '\0')
    {
        size_t password_size = strlen(password_buff);

        if(username_buff != NULL && username_buff[0] != '\0')
        {
            size_t username_size = strlen(username_buff);

            /*--------------------------------------------------------------------------------------------------------*/

            char cmd_buff[64 + username_size + password_size];

            /*--------------------------------------------------------------------------------------------------------*/

            size_t cmd_size = snprintf(
                /*--*/(cmd_buff),
                sizeof(cmd_buff),
                "*3\r\n"
                "$4\r\nAUTH\r\n"
                "$%zu\r\n%s\r\n"
                "$%zu\r\n%s\r\n",
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

            size_t cmd_size = snprintf(
                /*--*/(cmd_buff),
                sizeof(cmd_buff),
                "*2\r\n"
                "$4\r\nAUTH\r\n"
                "$%zu\r\n%s\r\n",
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
}

/*--------------------------------------------------------------------------------------------------------------------*/

void nyx_redis_pub(nyx_node_t *node, STR_t device, STR_t stream, size_t max_len, __ZEROABLE__ size_t n_fields, const str_t field_names[], const size_t field_sizes[], const buff_t field_buffs[])
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

    size_t header_size = snprintf(
        /*--*/(header_buff),
        sizeof(header_buff),
        "*%zu\r\n"
        "$4\r\nXADD\r\n"
        "$%zu\r\n%s/%s\r\n"
        "$6\r\nMAXLEN\r\n"
        "$1\r\n~\r\n"
        "$%zu\r\n%zu\r\n"
        "$1\r\n*\r\n",
        6 + 2 * n_fields,
        strlen(device) + 1 + strlen(stream),
        /*--*/(device)   ,   /*--*/(stream),
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
                /**/ if(field_name[0] == '%')
                {
                    value_buff = nyx_zlib_deflate(&value_size, field_size, field_buff);

                    field_name = field_name + 1;

                    allocated = true;
                }
                else if(field_name[0] == '#')
                {
                    value_buff = nyx_base64_encode(&value_size, field_size, field_buff);

                    field_name = field_name + 1;

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

            header_size = snprintf(
                /*--*/(header_buff),
                sizeof(header_buff),
                "$%zu\r\n%s\r\n"
                "$%zu\r\n",
                strlen(field_name),
                /*--*/(field_name),
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

bool nyx_stream_pub(nyx_node_t *node, STR_t device, STR_t stream, bool check, size_t max_len, __ZEROABLE__ size_t n_fields, const str_t field_names[], const size_t field_sizes[], const buff_t field_buffs[])
{
    /*----------------------------------------------------------------------------------------------------------------*/
    /* CHECK IF STREAM IS ENABLED                                                                                     */
    /*----------------------------------------------------------------------------------------------------------------*/

    if(check)
    {
        for(nyx_dict_t **vector_ptr = node->vectors; *vector_ptr != NULL; vector_ptr++)
        {
            nyx_dict_t *vector = *vector_ptr;

            /*--------------------------------------------------------------------------------------------------------*/

            STR_t vector_device = nyx_dict_get_string(vector, "@device");
            STR_t vector_stream = nyx_dict_get_string(vector,  "@name" );

            if(vector_device!= NULL && vector_stream != NULL && strcmp(vector_device, device) == 0 && strcmp(vector_stream, stream) == 0)
            {
                if((vector->base.flags & NYX_FLAGS_STREAM_MASK) == 0)
                {
                    return false;
                }

                break;
            }

            /*--------------------------------------------------------------------------------------------------------*/
        }
    }

    /*----------------------------------------------------------------------------------------------------------------*/
    /* PUBLISH STREAM                                                                                                 */
    /*----------------------------------------------------------------------------------------------------------------*/

    nyx_redis_pub(
        node,
        device,
        stream,
        max_len,
        n_fields,
        field_names,
        field_sizes,
        field_buffs
    );

    /*----------------------------------------------------------------------------------------------------------------*/

    return true;
}

/*--------------------------------------------------------------------------------------------------------------------*/
