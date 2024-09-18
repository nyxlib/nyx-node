/*--------------------------------------------------------------------------------------------------------------------*/

#include <string.h>

#include "nyx_node_internal.h"

/*--------------------------------------------------------------------------------------------------------------------*/

typedef struct nyx_string_builder_node_s
{
    bool xml;

    struct nyx_string_builder_node_s *next;

} node_t;

/*--------------------------------------------------------------------------------------------------------------------*/

nyx_string_builder_t *nyx_string_builder_new()
{
    /*----------------------------------------------------------------------------------------------------------------*/

    nyx_string_builder_t *sb = nyx_memory_alloc(sizeof(nyx_string_builder_t));

    /*----------------------------------------------------------------------------------------------------------------*/

    sb->head = NULL;
    sb->tail = NULL;

    /*----------------------------------------------------------------------------------------------------------------*/

    return sb;
}

/*--------------------------------------------------------------------------------------------------------------------*/

void nyx_string_builder_free(nyx_string_builder_t *sb)
{
    nyx_string_builder_clear(sb);

    nyx_memory_free(sb);
}

/*--------------------------------------------------------------------------------------------------------------------*/

void nyx_string_builder_clear(nyx_string_builder_t *sb)
{
    /*----------------------------------------------------------------------------------------------------------------*/

    for(node_t *node = sb->head; node != NULL;)
    {
        /*------------------------------------------------------------------------------------------------------------*/

        node_t *temp = node;

        node = node->next;

        /*------------------------------------------------------------------------------------------------------------*/

        nyx_memory_free(temp);

        /*------------------------------------------------------------------------------------------------------------*/
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    sb->head = NULL;
    sb->tail = NULL;

    /*----------------------------------------------------------------------------------------------------------------*/
}

/*--------------------------------------------------------------------------------------------------------------------*/

void nyx_string_builder_append_n(nyx_string_builder_t *sb, STR_t args[], size_t n, bool xml)
{
    for(size_t i = 0; i < n; i++)
    {
        /*------------------------------------------------------------------------------------------------------------*/

        STR_t data = args[i];

        /*------------------------------------------------------------------------------------------------------------*/

        node_t *node;

        if(data == NULL)
        {
            node = nyx_memory_alloc(sizeof(node_t) + 0x00000000000006 + 1);

            strcpy((str_t) (node + 1), "(null)");
        }
        else
        {
            node = nyx_memory_alloc(sizeof(node_t) + strlen(data) + 1);

            strcpy((str_t) (node + 1), data);
        }

        node->xml = xml;
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

size_t string_builder_length(const nyx_string_builder_t *sb, bool json_string)
{
    size_t result;

    /*----------------------------------------------------------------------------------------------------------------*/

    if(json_string)
    {
        /*--------------------------------------------------------------------------------------------------------*/
        /* JSON STRING                                                                                            */
        /*--------------------------------------------------------------------------------------------------------*/

        result = 2;

        for(node_t *node = sb->head; node != NULL; node = node->next)
        {
            str_t q = (str_t) (node + 1);

            size_t size = strlen(q);

            if(node->xml)
            {
                for(; size > 0; size--)
                {
                    switch(*q++)
                    {
                        case '<':
                        case '>':
                            result += 4;
                            break;

                        case '&':
                            result += 5;
                            break;

                        case '\"':
                        case '\'':
                            result += 6;
                            break;

                        case '\\':
                        case '\b':
                        case '\f':
                        case '\n':
                        case '\r':
                        case '\t':
                            result += 2;
                            break;

                        default:
                            result += 1;
                            break;
                    }
                }
            }
            else
            {
                for(; size > 0; size--)
                {
                    switch(*q++)
                    {
                        case '\"':
                        case '\\':
                        case '\b':
                        case '\f':
                        case '\n':
                        case '\r':
                        case '\t':
                            result += 2;
                            break;

                        default:
                            result += 1;
                            break;
                    }
                }
            }
        }

        /*--------------------------------------------------------------------------------------------------------*/
    }
    else
    {
        /*--------------------------------------------------------------------------------------------------------*/
        /* RAW STRING                                                                                             */
        /*--------------------------------------------------------------------------------------------------------*/

        result = 0;

        for(node_t *node = sb->head; node != NULL; node = node->next)
        {
            str_t q = (str_t) (node + 1);

            size_t size = strlen(q);

            if(node->xml)
            {
                for(; size > 0; size--)
                {
                    switch(*q++)
                    {
                        case '<':
                        case '>':
                            result += 4;
                            break;

                        case '&':
                            result += 5;
                            break;

                        case '\"':
                        case '\'':
                            result += 6;
                            break;

                        default:
                            result += 1;
                            break;
                    }
                }
            }
            else
            {
                result += size;
            }

            /*--------------------------------------------------------------------------------------------------------*/
        }
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    return result;
}

/*--------------------------------------------------------------------------------------------------------------------*/

size_t nyx_string_builder_length(const nyx_string_builder_t *sb)
{
    return string_builder_length(sb, true);
}

/*--------------------------------------------------------------------------------------------------------------------*/

size_t nyx_string_builder_clength(const nyx_string_builder_t *sb)
{
    return string_builder_length(sb, false);
}

/*--------------------------------------------------------------------------------------------------------------------*/

static str_t to_string(const nyx_string_builder_t *sb, bool json_string)
{
    /*----------------------------------------------------------------------------------------------------------------*/

    str_t result = nyx_memory_alloc( string_builder_length(sb, json_string) + 1), p = result;

    /*----------------------------------------------------------------------------------------------------------------*/

    char c;

    /*----------------------------------------------------------------------------------------------------------------*/

    if(json_string)
    {
        /*------------------------------------------------------------------------------------------------------------*/
        /* JSON STRING                                                                                                */
        /*------------------------------------------------------------------------------------------------------------*/

        *p++ = '"';

        /*------------------------------------------------------------------------------------------------------------*/

        for(node_t *node = sb->head; node != NULL; node = node->next)
        {
            str_t q = (str_t) (node + 1);

            size_t size = strlen(q);

            if(node->xml)
            {
                for(; size > 0; size--)
                {
                    switch((c = *q++))
                    {
                        case '<':
                            *p++ = '&'; *p++ = 'l'; *p++ = 't'; *p++ = ';'; break;

                        case '>':
                            *p++ = '&'; *p++ = 'g'; *p++ = 't'; *p++ = ';'; break;

                        case '&':
                            *p++ = '&'; *p++ = 'a'; *p++ = 'm'; *p++ = 'p'; *p++ = ';'; break;

                        case '\"':
                            *p++ = '&'; *p++ = 'q'; *p++ = 'u'; *p++ = 'o'; *p++ = 't'; *p++ = ';'; break;

                        case '\'':
                            *p++ = '&'; *p++ = 'a'; *p++ = 'p'; *p++ = 'o'; *p++ = 's'; *p++ = ';'; break;

                        case '\\': *p++ = '\\'; *p++ = '\\'; break;
                        case '\b': *p++ = '\\'; *p++ = 'b'; break;
                        case '\f': *p++ = '\\'; *p++ = 'f'; break;
                        case '\n': *p++ = '\\'; *p++ = 'n'; break;
                        case '\r': *p++ = '\\'; *p++ = 'r'; break;
                        case '\t': *p++ = '\\'; *p++ = 't'; break;

                        default:
                            *p++ = c;
                            break;
                    }
                }
            }
            else
            {
                for(; size > 0; size--)
                {
                    switch((c = *q++))
                    {
                        case '\"': *p++ = '\\'; *p++ = '\"'; break;
                        case '\\': *p++ = '\\'; *p++ = '\\'; break;
                        case '\b': *p++ = '\\'; *p++ = 'b'; break;
                        case '\f': *p++ = '\\'; *p++ = 'f'; break;
                        case '\n': *p++ = '\\'; *p++ = 'n'; break;
                        case '\r': *p++ = '\\'; *p++ = 'r'; break;
                        case '\t': *p++ = '\\'; *p++ = 't'; break;

                        default:
                            *p++ = c;
                            break;
                    }
                }
            }
        }

        /*------------------------------------------------------------------------------------------------------------*/

        *p++ = '"';

        /*------------------------------------------------------------------------------------------------------------*/
    }
    else
    {
        /*------------------------------------------------------------------------------------------------------------*/
        /* RAW STRING                                                                                                 */
        /*------------------------------------------------------------------------------------------------------------*/

        for(node_t *node = sb->head; node != NULL; node = node->next)
        {
            str_t q = (str_t) (node + 1);

            size_t size = strlen(q);

            if(node->xml)
            {
                for(; size > 0; size--)
                {
                    switch((c = *q++))
                    {
                        case '<':
                            *p++ = '&'; *p++ = 'l'; *p++ = 't'; *p++ = ';'; break;

                        case '>':
                            *p++ = '&'; *p++ = 'g'; *p++ = 't'; *p++ = ';'; break;

                        case '&':
                            *p++ = '&'; *p++ = 'a'; *p++ = 'm'; *p++ = 'p'; *p++ = ';'; break;

                        case '\"':
                            *p++ = '&'; *p++ = 'q'; *p++ = 'u'; *p++ = 'o'; *p++ = 't'; *p++ = ';'; break;

                        case '\'':
                            *p++ = '&'; *p++ = 'a'; *p++ = 'p'; *p++ = 'o'; *p++ = 's'; *p++ = ';'; break;

                        default:
                            *p++ = c;
                            break;
                    }
                }
            }
            else
            {
                p = memcpy(p, q, size) + size;
            }
        }

        /*------------------------------------------------------------------------------------------------------------*/
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    *p = '\0';

    /*----------------------------------------------------------------------------------------------------------------*/

    return result;
}

/*--------------------------------------------------------------------------------------------------------------------*/

str_t nyx_string_builder_to_string(const nyx_string_builder_t *sb)
{
    return to_string(sb, true);
}

/*--------------------------------------------------------------------------------------------------------------------*/

str_t nyx_string_builder_to_cstring(const nyx_string_builder_t *sb)
{
    return to_string(sb, false);
}

/*--------------------------------------------------------------------------------------------------------------------*/
