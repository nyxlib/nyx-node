/* NyxNode
 * Author: Jérôme ODIER <jerome.odier@lpsc.in2p3.fr>
 * SPDX-License-Identifier: GPL-2.0-only (Mongoose backend) or GPL-3.0+
 */

/*--------------------------------------------------------------------------------------------------------------------*/

#include <string.h>

#include "nyx_node_internal.h"

/*--------------------------------------------------------------------------------------------------------------------*/

static struct tag_def_s
{
    STR_t s_tag;
    STR_t e_tag;

} TAGS[] = {
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

#define TAG_DEF_NB (sizeof(TAGS) / sizeof(struct tag_def_s))

/*--------------------------------------------------------------------------------------------------------------------*/

bool nyx_stream_detect_opening_tag(nyx_stream_t *stream, size_t size, BUFF_t buff)
{
    for(int i = 0; i < TAG_DEF_NB; i++)
    {
        STR_t p = memmem(buff, size, TAGS[i].s_tag, strlen(TAGS[i].s_tag));

        if(p != NULL)
        {
            stream->idx = i;
            stream->s_ptr = p;

            stream->pos = (
                (size_t) stream->s_ptr
                -
                (size_t) /**/buff/**/
            );

            return true;
        }
    }

    return false;
}

/*--------------------------------------------------------------------------------------------------------------------*/

bool nyx_stream_detect_closing_tag(nyx_stream_t *stream, size_t size, __UNUSED__ BUFF_t buff)
{
    STR_t p = memmem(stream->s_ptr, size - stream->pos, TAGS[stream->idx].e_tag, strlen(TAGS[stream->idx].e_tag));

    if(p != NULL)
    {
        stream->e_ptr = p + strlen(TAGS[stream->idx].e_tag);

        stream->len = (
            (size_t) stream->e_ptr
            -
            (size_t) stream->s_ptr
        );

        return true;
    }

    return false;
}

/*--------------------------------------------------------------------------------------------------------------------*/
