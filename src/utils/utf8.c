/* NyxNode
 * Author: Jérôme ODIER <jerome.odier@lpsc.in2p3.fr>
 * SPDX-License-Identifier: GPL-2.0-only (Mongoose backend) or GPL-3.0+
 */

/*--------------------------------------------------------------------------------------------------------------------*/

#include "../nyx_node_internal.h"

/*--------------------------------------------------------------------------------------------------------------------*/

int nyx_unicode_to_utf8(str_t result, uint32_t unicode_char)
{
    uint8_t *up = (uint8_t *) result;

    /*----------------------------------------------------------------------------------------------------------------*/

    /* ignore surrogates */

    if(unicode_char >= 0xD800U
       &&
       unicode_char <= 0xDFFFU
    ) {
        goto _error;
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    if(unicode_char <= 0x7FU)
    {
        *up = (uint8_t) (0x00U | ((unicode_char >> 0) & 0x7FU));
        return 1;
    }

    if(unicode_char <= 0x7FFU)
    {
        *up++ = (uint8_t) (0xC0U | ((unicode_char >> 6) & 0x1FU));
        *up   = (uint8_t) (0x80U | ((unicode_char >> 0) & 0x3FU));
        return 2;
    }

    if(unicode_char <= 0xFFFFU)
    {
        *up++ = (uint8_t) (0xE0U | ((unicode_char >> 12) & 0x0FU));
        *up++ = (uint8_t) (0x80U | ((unicode_char >> 6) & 0x3FU));
        *up   = (uint8_t) (0x80U | ((unicode_char >> 0) & 0x3FU));
        return 3;
    }

    if(unicode_char <= 0x10FFFFU)
    {
        *up++ = (uint8_t) (0xF0U | ((unicode_char >> 18) & 0x07U));
        *up++ = (uint8_t) (0x80U | ((unicode_char >> 12) & 0x3FU));
        *up++ = (uint8_t) (0x80U | ((unicode_char >> 6) & 0x3FU));
        *up   = (uint8_t) (0x80U | ((unicode_char >> 0) & 0x3FU));
        return 4;
    }

    /*----------------------------------------------------------------------------------------------------------------*/

/* NOSONAR */ _error:
    *up = '?';

    return 1;
}

/*--------------------------------------------------------------------------------------------------------------------*/
