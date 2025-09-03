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
#ifdef HAVE_ZLIB
/*--------------------------------------------------------------------------------------------------------------------*/

static buff_t _internal_deflate(size_t *result_size, size_t uncomp_size, BUFF_t uncomp_buff)
{
    /*----------------------------------------------------------------------------------------------------------------*/

    uLongf comp_size = compressBound((uLong) uncomp_size);

    Bytef *comp_buff = (Bytef *) nyx_memory_alloc((size_t) comp_size);

    /*----------------------------------------------------------------------------------------------------------------*/

    int ret = compress2(comp_buff, &comp_size, (const Bytef *) uncomp_buff, (uLong) uncomp_size, Z_BEST_COMPRESSION);

    if(ret != Z_OK)
    {
        nyx_memory_free(comp_buff);

        *result_size = 0x00;

        return NULL;
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    *result_size = comp_size;

    return comp_buff;

    /*----------------------------------------------------------------------------------------------------------------*/
}

/*--------------------------------------------------------------------------------------------------------------------*/

static buff_t _internal_inflate(size_t *result_size, size_t comp_size, BUFF_t comp_buff)
{
    /*----------------------------------------------------------------------------------------------------------------*/

    uLongf uncomp_size = (uLongf) *result_size;

    Bytef *uncomp_buff = (Bytef *) nyx_memory_alloc(*result_size);

    /*----------------------------------------------------------------------------------------------------------------*/

    int ret = uncompress(uncomp_buff, &uncomp_size, (const Bytef *) comp_buff, (uLong) comp_size);

    if(ret != Z_OK)
    {
        nyx_memory_free(uncomp_buff);

        *result_size = 0x00;

        return false;
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    *result_size = uncomp_size;

    return uncomp_buff;

    /*----------------------------------------------------------------------------------------------------------------*/
}

/*--------------------------------------------------------------------------------------------------------------------*/
#else
/*--------------------------------------------------------------------------------------------------------------------*/

static buff_t _internal_deflate(size_t *result_size, size_t uncomp_size, BUFF_t uncomp_buff)
{
    *result_size = size;

    return buff;
}

/*--------------------------------------------------------------------------------------------------------------------*/

static buff_t _internal_inflate(size_t *result_size, size_t comp_size, BUFF_t comp_buff)
{
    *result_size = size;

    return buff;
}

/*--------------------------------------------------------------------------------------------------------------------*/
#endif
/*--------------------------------------------------------------------------------------------------------------------*/

str_t nyx_zlib_compress(__NULLABLE__ size_t *result_len, __ZEROABLE__ size_t size, __NULLABLE__ BUFF_t buff)
{
    if(size == 0x00 || buff == NULL)
    {
        if(result_len)
        {
            *result_len = 0x00;
        }

        return NULL;
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    size_t comp_size;
    buff_t comp_buff = _internal_deflate(&comp_size, size, buff);

    if(comp_size > 0x00 && comp_buff != NULL)
    {
        str_t result = nyx_base64_encode(result_len, comp_size, comp_buff);

        nyx_memory_free(comp_buff);

        return result;
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    if(result_len)
    {
        *result_len = 0x00;
    }

    return NULL;
}

/*--------------------------------------------------------------------------------------------------------------------*/

buff_t nyx_zlib_uncompress(size_t *result_size, __ZEROABLE__ size_t len, __NULLABLE__ STR_t str)
{
    if(result_size == NULL)
    {
        NYX_LOG_ERROR("Initial size not provided in `result_size`");

        return NULL;
    }

    if(len == 0x00 || str == NULL)
    {
        *result_size = 0x00;

        return NULL;
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    size_t comp_size;
    buff_t comp_buff = nyx_base64_decode(&comp_size, len, str);

    if(comp_size > 0x00 && comp_buff != NULL)
    {
        buff_t result = _internal_inflate(result_size, comp_size, comp_buff);

        nyx_memory_free(comp_buff);

        return result;
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    *result_size = 0x00;

    return NULL;
}

/*--------------------------------------------------------------------------------------------------------------------*/
