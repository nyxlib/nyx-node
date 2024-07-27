/*--------------------------------------------------------------------------------------------------------------------*/

#include <string.h>

#include "indi_node_internal.h"

/*--------------------------------------------------------------------------------------------------------------------*/

typedef struct indi_string_builder_node_s
{
    struct indi_string_builder_node_s *next;

} node_t;

/*--------------------------------------------------------------------------------------------------------------------*/

indi_string_builder_t *indi_string_builder_new()
{
    /*----------------------------------------------------------------------------------------------------------------*/

    indi_string_builder_t *sb = indi_memory_alloc(sizeof(indi_string_builder_t));

    /*----------------------------------------------------------------------------------------------------------------*/

    sb->head = NULL;
    sb->tail = NULL;

    /*----------------------------------------------------------------------------------------------------------------*/

    return sb;
}

/*--------------------------------------------------------------------------------------------------------------------*/

void indi_string_builder_free(indi_string_builder_t *sb)
{
    indi_string_builder_clear(sb);

    indi_memory_free(sb);
}

/*--------------------------------------------------------------------------------------------------------------------*/

void indi_string_builder_clear(indi_string_builder_t *sb)
{
    /*----------------------------------------------------------------------------------------------------------------*/

    for(node_t *node = sb->head; node != NULL;)
    {
        /*------------------------------------------------------------------------------------------------------------*/

        node_t *temp = node;

        node = node->next;

        /*------------------------------------------------------------------------------------------------------------*/

        indi_memory_free(temp);

        /*------------------------------------------------------------------------------------------------------------*/
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    sb->head = NULL;
    sb->tail = NULL;

    /*----------------------------------------------------------------------------------------------------------------*/
}

/*--------------------------------------------------------------------------------------------------------------------*/

void indi_string_builder_append_n(indi_string_builder_t *sb, STR_t args[], size_t n)
{
    for(size_t i = 0; i < n; i++)
    {
        /*------------------------------------------------------------------------------------------------------------*/

        STR_t data = args[i];

        /*------------------------------------------------------------------------------------------------------------*/

        node_t *node;

        if(data == NULL)
        {
            node = indi_memory_alloc(sizeof(node_t) + 0x00000000000006 + 1);

            strcpy((str_t) (node + 1), "(null)");
        }
        else
        {
            node = indi_memory_alloc(sizeof(node_t) + strlen(data) + 1);

            strcpy((str_t) (node + 1), data);
        }

        node->next = NULL;

        /*------------------------------------------------------------------------------------------------------------*/

        if(sb->head == NULL)
        {
            sb->head = node;
            sb->tail = node;
        }
        else
        {
            sb->tail->next = node;
            sb->tail /*-*/ = node;
        }

        /*------------------------------------------------------------------------------------------------------------*/
    }
}

/*--------------------------------------------------------------------------------------------------------------------*/

size_t indi_string_builder_length(indi_string_builder_t *sb)
{
    size_t length = 0;

    /*----------------------------------------------------------------------------------------------------------------*/

    for(node_t *node = sb->head; node != NULL; node = node->next)
    {
        length += strlen((str_t) (node + 1));
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    return length;
}

/*--------------------------------------------------------------------------------------------------------------------*/

static str_t to_string(indi_string_builder_t *sb, bool json_string)
{
    /*----------------------------------------------------------------------------------------------------------------*/

    size_t length = indi_string_builder_length(sb);

    /*----------------------------------------------------------------------------------------------------------------*/

    str_t result = indi_memory_alloc(json_string ? 2 * length + 3
                                                      : 1 * length + 1
    ), p = result;

    /*----------------------------------------------------------------------------------------------------------------*/

    if(json_string)
    {
        /*------------------------------------------------------------------------------------------------------------*/

        *p++ = '"';

        /*------------------------------------------------------------------------------------------------------------*/

        for(node_t *node = sb->head; node != NULL; node = node->next)
        {
            str_t q = (str_t) (node + 1);

            for(size_t size = strlen(q); size > 0; size--)
            {
                switch(*q)
                {
                    case '\"': *p++ = '\\'; *p++ = '\"'; break;
                    case '\\': *p++ = '\\'; *p++ = '\\'; break;
                    case '\b': *p++ = '\\'; *p++ = 'b'; break;
                    case '\f': *p++ = '\\'; *p++ = 'f'; break;
                    case '\n': *p++ = '\\'; *p++ = 'n'; break;
                    case '\r': *p++ = '\\'; *p++ = 'r'; break;
                    case '\t': *p++ = '\\'; *p++ = 't'; break;
                    default:
                        *p++ = *q;
                        break;
                }

                q++;
            }
        }

        /*------------------------------------------------------------------------------------------------------------*/

        *p++ = '"';

        /*------------------------------------------------------------------------------------------------------------*/
    }
    else
    {
        /*------------------------------------------------------------------------------------------------------------*/

        for(node_t *node = sb->head; node != NULL; node = node->next)
        {
            size_t size = strlen((str_t) (node + 1));

            memcpy(p, (str_t) (node + 1), size);

            p += size;
        }

        /*------------------------------------------------------------------------------------------------------------*/
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    *p = '\0';

    /*----------------------------------------------------------------------------------------------------------------*/

    return result;
}

/*--------------------------------------------------------------------------------------------------------------------*/

str_t indi_string_builder_to_string(indi_string_builder_t *sb)
{
    return to_string(sb, true);
}

/*--------------------------------------------------------------------------------------------------------------------*/

str_t indi_string_builder_to_cstring(indi_string_builder_t *sb)
{
    return to_string(sb, false);
}

/*--------------------------------------------------------------------------------------------------------------------*/
