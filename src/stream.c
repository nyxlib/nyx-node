/*--------------------------------------------------------------------------------------------------------------------*/

#include <string.h>

#include "indi_node_internal.h"

/*--------------------------------------------------------------------------------------------------------------------*/

struct tag_def_s
{
    STR_t s_tag;
    STR_t e_tag;
};

/*--------------------------------------------------------------------------------------------------------------------*/

struct tag_def_s TAGS[] = {
    {.s_tag = "<getProperties", .e_tag = "/>"},
    {.s_tag = "<delProperty", .e_tag = "/>"},
    {.s_tag = "<message", .e_tag = "/>"},
    /**/
    {.s_tag = "<enableBLOB", .e_tag = "</enableBLOB>"},
    /**/
    {.s_tag = "<newTextVector", .e_tag = "</newTextVector>"},
    {.s_tag = "<newNumberVector", .e_tag = "</newNumberVector>"},
    {.s_tag = "<newSwitchVector", .e_tag = "</newSwitchVector>"},
    {.s_tag = "<newLightVector", .e_tag = "</newLightVector>"},
    {.s_tag = "<newBLOBVector", .e_tag = "</newBLOBVector>"},
};

/*--------------------------------------------------------------------------------------------------------------------*/

#define TAG_NB (sizeof(TAGS) / sizeof(struct tag_def_s))

/*--------------------------------------------------------------------------------------------------------------------*/

bool indi_stream_detect_opening_tag(tag_t *tag, size_t size, BUFF_t buff)
{
    for(int i = 0; i < TAG_NB; i++)
    {
        STR_t p = strnstr(buff, TAGS[i].s_tag, size);

        if(p != NULL)
        {
            tag->idx = i;
            tag->s_ptr = p;

            tag->pos = (size_t) tag->s_ptr - (size_t) buff;

            return true;
        }
    }

    return false;
}

/*--------------------------------------------------------------------------------------------------------------------*/

bool indi_stream_detect_closing_tag(tag_t *tag, size_t size, BUFF_t buff)
{
    STR_t p = strnstr(tag->s_ptr, TAGS[tag->idx].e_tag, size - tag->pos);

    if(p != NULL)
    {
        tag->e_ptr = p + strlen(TAGS[tag->idx].e_tag);

        tag->len = (size_t) tag->e_ptr - (size_t) tag->s_ptr;

        return true;
    }

    return false;
}

/*--------------------------------------------------------------------------------------------------------------------*/
