/*--------------------------------------------------------------------------------------------------------------------*/

#include "nyx_node_internal.h"

/*--------------------------------------------------------------------------------------------------------------------*/

int nyx_unicode_to_utf8(uint32_t unicode_char, str_t p)
{
    uint8_t *up = (uint8_t *) p;

    /**/ if(unicode_char <= 0x7F)
    {
        *up = unicode_char;
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
