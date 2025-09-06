/* NyxNode
 * Author: Jérôme ODIER <jerome.odier@lpsc.in2p3.fr>
 * SPDX-License-Identifier: GPL-2.0-only (Mongoose backend) or GPL-3.0+
 */

/*--------------------------------------------------------------------------------------------------------------------*/

#ifdef HAVE_ZLIB
#  include <zlib.h>
#else
#  include <string.h>
#endif

#include "../nyx_node_internal.h"

/*--------------------------------------------------------------------------------------------------------------------*/
#ifdef HAVE_ZLIB
/*--------------------------------------------------------------------------------------------------------------------*/

buff_t nyx_zlib_compress(__NULLABLE__ size_t *result_size, __ZEROABLE__ size_t size, __NULLABLE__ BUFF_t buff)
{
    if(size == 0x00 || buff == NULL)
    {
        if(result_size != NULL)
        {
            *result_size = 0x00;
        }

        return NULL;
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    uLongf comp_size = compressBound((uLong) size);

    Bytef *comp_buff = (Bytef *) nyx_memory_alloc((size_t) comp_size);

    /*----------------------------------------------------------------------------------------------------------------*/

    int ret = compress2(comp_buff, &comp_size, (const Bytef *) buff, (uLong) size, Z_BEST_COMPRESSION);

    if(ret != Z_OK)
    {
        nyx_memory_free(comp_buff);

        if(result_size != NULL)
        {
            *result_size = 0x00;
        }

        return NULL;
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    if(result_size != NULL)
    {
        *result_size = comp_size;
    }

    return comp_buff;

    /*----------------------------------------------------------------------------------------------------------------*/
}

/*--------------------------------------------------------------------------------------------------------------------*/

buff_t nyx_zlib_uncompress(__NOTNULL__ size_t *result_size, __ZEROABLE__ size_t size, __NULLABLE__ BUFF_t buff)
{
    if(result_size == NULL)
    {
        NYX_LOG_ERROR("Initial size not provided in `result_size`");

        return NULL;
    }

    if(size == 0x00 || buff == NULL)
    {
        //(result_size != NULL)
        {
            *result_size = 0x00;
        }

        return NULL;
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    uLongf uncomp_size = (uLongf) *result_size;

    Bytef *uncomp_buff = (Bytef *) nyx_memory_alloc(*result_size);

    /*----------------------------------------------------------------------------------------------------------------*/

    int ret = uncompress(uncomp_buff, &uncomp_size, (const Bytef *) buff, (uLong) size);

    if(ret != Z_OK)
    {
        nyx_memory_free(uncomp_buff);

        //(result_size != NULL)
        {
            *result_size = 0x00;
        }

        return NULL;
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    //(result_size != NULL)
    {
        *result_size = uncomp_size;
    }

    return uncomp_buff;

    /*----------------------------------------------------------------------------------------------------------------*/
}

/*--------------------------------------------------------------------------------------------------------------------*/
#else
/*--------------------------------------------------------------------------------------------------------------------*/

static const uint32_t ADLER_MOD = 65521u;

static const uint32_t ADLER_N_MAX = 5552u;

/*--------------------------------------------------------------------------------------------------------------------*/

uint32_t internal_adler32(size_t src_size, BUFF_t src_buff)
{
    /*----------------------------------------------------------------------------------------------------------------*/

    const uint8_t *p = (const uint8_t *) src_buff;

    /*----------------------------------------------------------------------------------------------------------------*/

    uint32_t a = 1u;
    uint32_t b = 0u;

    while(src_size > 0)
    {
        size_t t = ADLER_N_MAX < src_size ? ADLER_N_MAX : src_size;

        src_size -= t;

        for(; t >= 4; t -= 4)
        {
            a += *p++; b += a;
            a += *p++; b += a;
            a += *p++; b += a;
            a += *p++; b += a;
        }

        for(; t > 0; t -= 1)
        {
            a += *p++; b += a;
        }

        a %= ADLER_MOD;
        b %= ADLER_MOD;
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    return (b << 16) | (a << 0);
}

/*--------------------------------------------------------------------------------------------------------------------*/

buff_t nyx_zlib_compress(__NULLABLE__ size_t *result_size, __ZEROABLE__ size_t size, BUFF_t buff)
{
    if(size == 0x00 || buff == NULL)
    {
        if(result_size != NULL)
        {
            *result_size = 0x00;
        }

        return NULL;
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    size_t n_blocks = size > 0 ? (size + 65534u) / 65535u : 1;

    size_t dst_size = 2 + n_blocks * 5 + size + 4;

    /*----------------------------------------------------------------------------------------------------------------*/

    const uint8_t *src = (const uint8_t *) /*------------*/(  buff  );
    /*-*/ uint8_t *dst = (/*-*/ uint8_t *) nyx_memory_alloc(dst_size);

    uint8_t *result_buff = dst;

    /*----------------------------------------------------------------------------------------------------------------*/

    *dst++ = 0x78;
    *dst++ = 0x01;

    /*----------------------------------------------------------------------------------------------------------------*/

    size_t rem = size;

    do {
        size_t chunk = 65535u < rem ? 65535u : rem;

        /* LAST */

        *dst++ = (uint8_t) (chunk == rem);

        /* LENGTH */

        *dst++ = (uint8_t) (chunk & 0xFF);
        *dst++ = (uint8_t) (chunk >> 8);
        *dst++ = (uint8_t) ((~chunk) & 0xFF);
        *dst++ = (uint8_t) ((~chunk) >> 8);

        /* PAYLOAD */

        if(chunk > 0)
        {
            memcpy(dst, src, chunk);

            dst += chunk;
            src += chunk;
            rem -= chunk;
        }

    } while(rem > 0);

    /*----------------------------------------------------------------------------------------------------------------*/

    uint32_t hash = internal_adler32(size, buff);

    *dst++ = (uint8_t) (hash >> 24);
    *dst++ = (uint8_t) (hash >> 16);
    *dst++ = (uint8_t) (hash >> 8);
    *dst++ = (uint8_t) (hash >> 0);

    /*----------------------------------------------------------------------------------------------------------------*/

    if(result_size != NULL)
    {
        *result_size = (size_t) (dst - result_buff);
    }

    return (buff_t) result_buff;
}

/*--------------------------------------------------------------------------------------------------------------------*/

buff_t nyx_zlib_uncompress(__NOTNULL__ size_t *result_size, __UNUSED__ size_t size, __UNUSED__ BUFF_t buff)
{
    NYX_LOG_ERROR("ZLib uncompression not supported");

    if(result_size != NULL)
    {
        *result_size = 0x00;
    }

    return NULL;
}

/*--------------------------------------------------------------------------------------------------------------------*/
#endif
/*--------------------------------------------------------------------------------------------------------------------*/

str_t nyx_zlib_base64_compress(__NULLABLE__ size_t *result_len, __ZEROABLE__ size_t size, __NULLABLE__ BUFF_t buff)
{
    /*----------------------------------------------------------------------------------------------------------------*/

    size_t comp_size;
    buff_t comp_buff = nyx_zlib_compress(&comp_size, size, buff);

    if(comp_size > 0x00 && comp_buff != NULL)
    {
        str_t result_str = nyx_base64_encode(result_len, comp_size, comp_buff);

        nyx_memory_free(comp_buff);

        return result_str;
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    if(result_len != NULL)
    {
        *result_len = 0x00;
    }

    return NULL;
}

/*--------------------------------------------------------------------------------------------------------------------*/

buff_t nyx_zlib_base64_uncompress(__NOTNULL__ size_t *result_size, __ZEROABLE__ size_t len, __NULLABLE__ STR_t str)
{
    /*----------------------------------------------------------------------------------------------------------------*/

    size_t comp_size;
    buff_t comp_buff = nyx_base64_decode(&comp_size, len, str);

    if(comp_size > 0x00 && comp_buff != NULL)
    {
        buff_t result_buff = nyx_zlib_uncompress(result_size, comp_size, comp_buff);

        nyx_memory_free(comp_buff);

        return result_buff;
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    if(result_size != NULL)
    {
        *result_size = 0x00;
    }

    return NULL;
}

/*--------------------------------------------------------------------------------------------------------------------*/
