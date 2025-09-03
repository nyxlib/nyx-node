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

static buff_t internal_deflate(size_t *result_size, size_t uncomp_size, BUFF_t uncomp_buff)
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

static buff_t internal_inflate(size_t *result_size, size_t comp_size, BUFF_t comp_buff)
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

        return NULL;
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    *result_size = uncomp_size;

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

    /*----------------------------------------------------------------------------------------------------------------*/

    while(src_size > 0)
    {
        size_t t = (src_size > ADLER_N_MAX) ? ADLER_N_MAX : src_size;

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

static buff_t internal_deflate(size_t *result_size, size_t src_size, BUFF_t src_buff)
{
    /*----------------------------------------------------------------------------------------------------------------*/

    size_t n_blocks = (src_size > 0) ? ((src_size + 65534u) / 65535u) : 1;

    size_t dst_size = 2 + n_blocks * 5 + src_size + 4;

    /*----------------------------------------------------------------------------------------------------------------*/

    const uint8_t *src = (const uint8_t *) /*------------*/(src_buff);
    /*-*/ uint8_t *dst = (/*-*/ uint8_t *) nyx_memory_alloc(dst_size);

    uint8_t *dst_buff = dst;

    /*----------------------------------------------------------------------------------------------------------------*/

    *dst++ = 0x78;
    *dst++ = 0x01;

    /*----------------------------------------------------------------------------------------------------------------*/

    size_t rem = src_size;

    do {
        size_t chunk = 65535u < rem ? 65535u
                                    : rem
        ;

        uint16_t len = (uint16_t) chunk;
        uint16_t nlen = (uint16_t) (~len);

        /* LAST */

        *dst++ = (uint8_t) (chunk == rem);

        /* LENGTH */

        *dst++ = (uint8_t) (len & 0xFF);
        *dst++ = (uint8_t) (len >> 8);
        *dst++ = (uint8_t) (nlen & 0xFF);
        *dst++ = (uint8_t) (nlen >> 8);

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

    uint32_t hash = internal_adler32(src_size, src_buff);

    *dst++ = (uint8_t) (hash >> 24);
    *dst++ = (uint8_t) (hash >> 16);
    *dst++ = (uint8_t) (hash >> 8);
    *dst++ = (uint8_t) (hash >> 0);

    /*----------------------------------------------------------------------------------------------------------------*/

    *result_size = (size_t) (dst - dst_buff);

    return (buff_t) dst_buff;
}

/*--------------------------------------------------------------------------------------------------------------------*/

static buff_t internal_inflate(size_t *result_size, size_t comp_size, BUFF_t comp_buff)
{
    NYX_LOG_ERROR("ZLib uncompress not supported");

    *result_size = comp_size;

    return comp_buff;
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
    buff_t comp_buff = internal_deflate(&comp_size, size, buff);

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
        buff_t result = internal_inflate(result_size, comp_size, comp_buff);

        nyx_memory_free(comp_buff);

        return result;
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    *result_size = 0x00;

    return NULL;
}

/*--------------------------------------------------------------------------------------------------------------------*/
