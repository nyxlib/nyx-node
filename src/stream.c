/*--------------------------------------------------------------------------------------------------------------------*/

#include <string.h>

#include "indi_node_internal.h"

/*--------------------------------------------------------------------------------------------------------------------*/

static char *_strnstr(const char *s, const char *find, size_t slen)
{
    char c, sc;
    size_t len;

    if ((c = *find++) != '\0') {
        len = strlen(find);
        do {
            do {
                if (slen-- < 1 || (sc = *s++) == '\0')
                    return (NULL);
            } while (sc != c);
            if (len > slen)
                return (NULL);
        } while (strncmp(s, find, len) != 0);
        s--;
    }
    return ((char *)s);
}

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

bool indi_stream_detect_opening_tag(indi_stream_t *stream, size_t size, BUFF_t buff)
{
    for(int i = 0; i < TAG_DEF_NB; i++)
    {
        STR_t p = _strnstr(buff, TAGS[i].s_tag, size);

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

bool indi_stream_detect_closing_tag(indi_stream_t *stream, size_t size, BUFF_t buff)
{
    STR_t p = _strnstr(stream->s_ptr, TAGS[stream->idx].e_tag, size - stream->pos);

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
