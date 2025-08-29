/* NyxNode
 * Author: Jérôme ODIER <jerome.odier@lpsc.in2p3.fr>
 * SPDX-License-Identifier: GPL-2.0-only (Mongoose backend) or GPL-3.0+
 */

/*--------------------------------------------------------------------------------------------------------------------*/

#ifdef HAVE_ZLIB
#include <zlib.h>
#endif

#include "../nyx_node_internal.h"

/*--------------------------------------------------------------------------------------------------------------------*/

str_t nyx_zlib_compress(__NULLABLE__ size_t *result_len, __ZEROABLE__ size_t size, __NULLABLE__ BUFF_t buff)
{
#ifndef HAVE_ZLIB
    return nyx_base64_encode(result_len, size, buff);
#else
    if(size == 0x00 || buff == NULL)
    {
        if(result_len)
        {
            *result_len = 0x00;
        }

        return NULL;
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    uLongf comp_size = compressBound((uLong) size);

    Bytef *comp_buff = (Bytef *) nyx_memory_alloc((size_t) comp_size);

    if(comp_buff == NULL)
    {
        if(result_len)
        {
            *result_len = 0x00;
        }

        return NULL;
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    int ret = compress2(comp_buff, &comp_size, (Bytef *) buff, (uLong) size, Z_BEST_COMPRESSION);

    ///_memory_free(comp_buff);

    if(ret != Z_OK)
    {
        nyx_memory_free(comp_buff);

        if(result_len)
        {
            *result_len = 0x00;
        }

        return NULL;
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    size_t len;
    str_t str = nyx_base64_encode(&len, (size_t) comp_size, (BUFF_t) comp_buff);

    nyx_memory_free(comp_buff);

    if(str == NULL)
    {
        ///_memory_free(comp_buff);

        if(result_len)
        {
            *result_len = 0x00;
        }

        return NULL;
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    if(result_len)
    {
        *result_len = len;
    }

    return str;
#endif
}

/*--------------------------------------------------------------------------------------------------------------------*/

buff_t nyx_zlib_uncompress(__NULLABLE__ size_t *result_size, __ZEROABLE__ size_t raw_size, __ZEROABLE__ size_t len, __NULLABLE__ STR_t str)
{
#ifndef HAVE_ZLIB
    return nyx_base64_decode(result_size, len, str);
#else
    if(raw_size == 0x00 || len == 0x00 || str == NULL)
    {
        if(result_size)
        {
            *result_size = 0x00;
        }

        return NULL;
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    size_t comp_size;
    buff_t comp_buff = nyx_base64_decode(&comp_size, len, str);

    if(comp_size == 0x00 || comp_buff == NULL)
    {
        if(result_size)
        {
            *result_size = 0x00;
        }

        return NULL;
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    uLongf size = (uLongf) raw_size;

    Bytef *buff = (Bytef *) nyx_memory_alloc(raw_size);

    int ret = uncompress(buff, &size, (Bytef *) comp_buff, (uLong) comp_size);

    nyx_memory_free(comp_buff);

    if(ret != Z_OK)
    {
        nyx_memory_free(buff);

        if(result_size)
        {
            *result_size = 0x00;
        }

        return NULL;
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    if(result_size)
    {
        *result_size = (size_t) size;
    }

    return (buff_t) buff;
#endif
}

/*--------------------------------------------------------------------------------------------------------------------*/
