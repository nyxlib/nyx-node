/*--------------------------------------------------------------------------------------------------------------------*/

#include "nyx_node.h"

/*--------------------------------------------------------------------------------------------------------------------*/

static const /*----*/ char BASE64_ENCODE_TABLE[] = {
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
    'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
    '+', '/',
};
/*--------------------------------------------------------------------------------------------------------------------*/

static const unsigned char BASE64_DECODE_TABLE[] = {
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
    64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 62, 64, 64, 64, 63, 52,
    53, 54, 55, 56, 57, 58, 59, 60, 61, 64, 64, 64,  0, 64, 64, 64,
    64,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
    15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 64, 64, 64, 64, 64,
    64, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
    41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 64, 64, 64, 64, 64,
};

/*--------------------------------------------------------------------------------------------------------------------*/

str_t nyx_base64_encode(__NULLABLE__ size_t *result_len, size_t size, BUFF_t buff)
{
    /*----------------------------------------------------------------------------------------------------------------*/

    size_t len = 4 * ((size + 2) / 3);

    str_t str = (str_t) nyx_memory_alloc(len + 1);

    /*----------------------------------------------------------------------------------------------------------------*/

    size_t div = size / 3;
    size_t mod = size % 3;

    const unsigned char * __restrict__ p = buff;
    /*-*/ /*----*/ char * __restrict__ q = str;

    /*----------------------------------------------------------------------------------------------------------------*/

    for(size_t i = 0; i < div; i++)
    {
        uint32_t triple = (
            (p[0] << 16)
            |
            (p[1] << 8)
            |
            (p[2] << 0)
        );

        *q++ = BASE64_ENCODE_TABLE[(triple >> 18) & 0x3F];
        *q++ = BASE64_ENCODE_TABLE[(triple >> 12) & 0x3F];
        *q++ = BASE64_ENCODE_TABLE[(triple >> 6) & 0x3F];
        *q++ = BASE64_ENCODE_TABLE[(triple >> 0) & 0x3F];

        p += 3;
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    /**/ if(mod == 2)
    {
        uint32_t triple = (
            (p[0] << 16)
            |
            (p[1] << 8)
        );

        *q++ = BASE64_ENCODE_TABLE[(triple >> 18) & 0x3F];
        *q++ = BASE64_ENCODE_TABLE[(triple >> 12) & 0x3F];
        *q++ = BASE64_ENCODE_TABLE[(triple >> 6) & 0x3F];
        *q++ = '=';
    }
    else if(mod == 1)
    {
        uint32_t triple = (
            (p[0] << 16)
        );

        *q++ = BASE64_ENCODE_TABLE[(triple >> 18) & 0x3F];
        *q++ = BASE64_ENCODE_TABLE[(triple >> 12) & 0x3F];
        *q++ = '=';
        *q++ = '=';
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    *q = '\0';

    /*----------------------------------------------------------------------------------------------------------------*/

    if(result_len)
    {
        *result_len = len;
    }

    return str;
}

/*--------------------------------------------------------------------------------------------------------------------*/

buff_t nyx_base64_decode(__NULLABLE__ size_t *result_size, size_t len, STR_t str)
{
    /*----------------------------------------------------------------------------------------------------------------*/

    int pad = (str[len - 1] == '=')
              +
              (str[len - 2] == '=')
    ;

    /*----------------------------------------------------------------------------------------------------------------*/

    size_t size = (len / 4) * 3 - pad;

    buff_t buff = (str_t) nyx_memory_alloc(size + 1);

    /*----------------------------------------------------------------------------------------------------------------*/

    size_t blocks = (len / 4) - (
        pad > 0 ? 1 : 0
    );

    const /*----*/ char * __restrict__ p = str;
    /*-*/ unsigned char * __restrict__ q = buff;

    /*----------------------------------------------------------------------------------------------------------------*/

    for(size_t i = 0; i < blocks; i++)
    {
        uint32_t triple = (
            (BASE64_DECODE_TABLE[(int) p[0]] << 18)
            |
            (BASE64_DECODE_TABLE[(int) p[1]] << 12)
            |
            (BASE64_DECODE_TABLE[(int) p[2]] << 6)
            |
            (BASE64_DECODE_TABLE[(int) p[3]] << 0)
        );

        *q++ = (triple >> 16) & 0xFF;
        *q++ = (triple >> 8) & 0xFF;
        *q++ = (triple >> 0) & 0xFF;

        p += 4;
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    /**/ if(pad == 1)
    {
        uint32_t triple = (
            (BASE64_DECODE_TABLE[(int) p[0]] << 18)
            |
            (BASE64_DECODE_TABLE[(int) p[1]] << 12)
            |
            (BASE64_DECODE_TABLE[(int) p[2]] << 6)
        );

        *q++ = (triple >> 16) & 0xFF;
        *q++ = (triple >> 8) & 0xFF;
    }
    else if(pad == 2)
    {
        uint32_t triple = (
            (BASE64_DECODE_TABLE[(int) p[0]] << 18)
            |
            (BASE64_DECODE_TABLE[(int) p[1]] << 12)
        );

        *q++ = (triple >> 16) & 0xFF;
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    *q = '\0';

    /*----------------------------------------------------------------------------------------------------------------*/

    if(result_size)
    {
        *result_size = size;
    }

    return buff;
}

/*--------------------------------------------------------------------------------------------------------------------*/
