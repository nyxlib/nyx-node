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

    /**/ if(unicode_char <= 0x7F)
    {
        *up = unicode_char & 0xFF;
        return 1;
    }
    else if(unicode_char <= 0x7FF)
    {
        *up++ = 0xC0 | ((unicode_char >> 6) & 0x1F);
        *up   = 0x80 | ((unicode_char >> 0) & 0x3F);
        return 2;
    }
    else if(unicode_char <= 0xFFFF)
    {
        *up++ = 0xE0 | ((unicode_char >> 12) & 0x0F);
        *up++ = 0x80 | ((unicode_char >> 6) & 0x3F);
        *up   = 0x80 | ((unicode_char >> 0) & 0x3F);
        return 3;
    }
    else
    {
        *up++ = 0xF0 | ((unicode_char >> 18) & 0x07);
        *up++ = 0x80 | ((unicode_char >> 12) & 0x3F);
        *up++ = 0x80 | ((unicode_char >> 6) & 0x3F);
        *up   = 0x80 | ((unicode_char >> 0) & 0x3F);
        return 4;
    }
}

/*--------------------------------------------------------------------------------------------------------------------*/
