/* NyxNode
 * Author: Jérôme ODIER <jerome.odier@lpsc.in2p3.fr>
 * SPDX-License-Identifier: GPL-2.0-only (Mongoose backend) or GPL-3.0+
 */

/*--------------------------------------------------------------------------------------------------------------------*/
#if !defined(ARDUINO)
/*--------------------------------------------------------------------------------------------------------------------*/

#include <string.h>

#include "nyx_node_internal.h"

/*--------------------------------------------------------------------------------------------------------------------*/

#define TAG(s_tag, e_tag) {                     \
            .s_tag_size = sizeof(s_tag) - 1,    \
            .s_tag_buff = (s_tag),              \
            .e_tag_size = sizeof(e_tag) - 1,    \
            .e_tag_buff = (e_tag),              \
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

bool nyx_xml_stream_detect_opening_tag(nyx_xml_stream_t *xml_stream, size_t size, BUFF_t buff)
{
    for(size_t i = 0; i < TAG_DEF_NB; i++)
    {
        STR_t p = memmem(buff, size, TAGS[i].s_tag_buff, TAGS[i].s_tag_size);

        if(p != NULL)
        {
            xml_stream->s_ptr = p + 0x000000000000000000000;

            xml_stream->pos = (
                (size_t) xml_stream->s_ptr
                -
                (size_t) /*--*/buff/*--*/
            );

            xml_stream->tag = &TAGS[i];

            return true;
        }
    }

    return false;
}

/*--------------------------------------------------------------------------------------------------------------------*/

bool nyx_xml_stream_detect_closing_tag(nyx_xml_stream_t *xml_stream, size_t size, __NYX_UNUSED__ BUFF_t buff)
{
    STR_t p = memmem(xml_stream->s_ptr, size - xml_stream->pos, xml_stream->tag->e_tag_buff, xml_stream->tag->e_tag_size);

    if(p != NULL)
    {
        xml_stream->e_ptr = p + xml_stream->tag->e_tag_size;

        xml_stream->len = (
            (size_t) xml_stream->e_ptr
            -
            (size_t) xml_stream->s_ptr
        );

        xml_stream->tag = NULL;

        return true;
    }

    return false;
}

/*--------------------------------------------------------------------------------------------------------------------*/
#endif
/*--------------------------------------------------------------------------------------------------------------------*/
