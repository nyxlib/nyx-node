/* NyxNode
 * Author: Jérôme ODIER <jerome.odier@lpsc.in2p3.fr>
 * SPDX-License-Identifier: GPL-2.0-only (Mongoose backend) or GPL-3.0+
 */

/*--------------------------------------------------------------------------------------------------------------------*/

#include <string.h>

#include "nyx_node_internal.h"

/*--------------------------------------------------------------------------------------------------------------------*/

#define TAG(s_tag, e_tag) {                     \
            .s_tag_size = sizeof(s_tag) - 1,    \
            .s_tag_buff = s_tag,                \
            .e_tag_size = sizeof(e_tag) - 1,    \
            .e_tag_buff = e_tag,                \
        }

/*--------------------------------------------------------------------------------------------------------------------*/

static struct tag_s
{
    size_t s_tag_size;
    STR_t s_tag_buff;
    size_t e_tag_size;
    STR_t e_tag_buff;

} TAGS[] = {
    TAG("<getProperties", "/>"),
    TAG("<delProperty", "/>"),
    TAG("<message", "/>"),
    /**/
    TAG("<enableBLOB", "</enableBLOB>"),
    /**/
    TAG("<newTextVector", "</newTextVector>"),
    TAG("<newNumberVector", "</newNumberVector>"),
    TAG("<newSwitchVector", "</newSwitchVector>"),
    TAG("<newLightVector", "</newLightVector>"),
    TAG("<newBLOBVector", "</newBLOBVector>"),
};

/*--------------------------------------------------------------------------------------------------------------------*/

#define TAG_DEF_NB (sizeof(TAGS) / sizeof(struct tag_s))

/*--------------------------------------------------------------------------------------------------------------------*/

bool nyx_stream_detect_opening_tag(nyx_stream_t *stream, size_t size, BUFF_t buff)
{
    for(int i = 0; i < TAG_DEF_NB; i++)
    {
        STR_t p = memmem(buff, size, TAGS[i].s_tag_buff, TAGS[i].s_tag_size);

        if(p != NULL)
        {
            stream->s_ptr = p + 0x000000000000000000000;

            stream->pos = (
                (size_t) stream->s_ptr
                -
                (size_t) /**/buff/**/
            );

            stream->tag = &TAGS[i];

            return true;
        }
    }

    return false;
}

/*--------------------------------------------------------------------------------------------------------------------*/

bool nyx_stream_detect_closing_tag(nyx_stream_t *stream, size_t size, __UNUSED__ BUFF_t buff)
{
    STR_t p = memmem(stream->s_ptr, size - stream->pos, stream->tag->e_tag_buff, stream->tag->e_tag_size);

    if(p != NULL)
    {
        stream->e_ptr = p + stream->tag->e_tag_size;

        stream->len = (
            (size_t) stream->e_ptr
            -
            (size_t) stream->s_ptr
        );

        stream->tag = NULL;

        return true;
    }

    return false;
}

/*--------------------------------------------------------------------------------------------------------------------*/
