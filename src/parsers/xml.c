/* NyxNode
 * Author: Jérôme ODIER <jerome.odier@lpsc.in2p3.fr>
 * SPDX-License-Identifier: GPL-2.0-only (Mongoose backend) or GPL-3.0+
 */

/*--------------------------------------------------------------------------------------------------------------------*/
#if !defined(ARDUINO)
/*--------------------------------------------------------------------------------------------------------------------*/

#include <ctype.h>
#include <string.h>

#include "../nyx_node_internal.h"

/*--------------------------------------------------------------------------------------------------------------------*/
/* DEFINITIONS                                                                                                        */
/*--------------------------------------------------------------------------------------------------------------------*/

typedef enum
{
    XML_TOKEN_EOF,
    XML_TOKEN_LT1,
    XML_TOKEN_LT2,
    XML_TOKEN_GT,
    XML_TOKEN_SLASH,
    XML_TOKEN_EQUALS,
    XML_TOKEN_IDENT,
    XML_TOKEN_STRING,
    XML_TOKEN_COMMENT,
    XML_TOKEN_CDATA,
    XML_TOKEN_TEXT,
    XML_TOKEN_ERROR,

} xml_token_type_t;

/*--------------------------------------------------------------------------------------------------------------------*/

typedef struct
{
    str_t value;

    xml_token_type_t token_type;

} xml_token_t;

/*--------------------------------------------------------------------------------------------------------------------*/

typedef struct
{
    bool tag;

    size_t size;
    STR_t buff;

    xml_token_t curr_token;

} xml_parser_t;

/*--------------------------------------------------------------------------------------------------------------------*/
/* TOKENIZER                                                                                                          */
/*--------------------------------------------------------------------------------------------------------------------*/

#define NEXT() \
            tokenizer_next(parser)

/*--------------------------------------------------------------------------------------------------------------------*/

#define PEEK() \
            (parser->curr_token)

/*--------------------------------------------------------------------------------------------------------------------*/

#define CHECK(t) \
            (parser->curr_token.token_type == (t))

/*--------------------------------------------------------------------------------------------------------------------*/

static const int XML_IDENT_TAB[256] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0,
    0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1,
    0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

/*--------------------------------------------------------------------------------------------------------------------*/

static bool xmlcpy(str_t p, STR_t s, STR_t e)
{
    while(s < e)
    {
        if(*s == '&')
        {
            s++;

            size_t size = (size_t) e - (size_t) s;

            /**/ if(size >= 3 && strncmp(s, "lt;", 3) == 0)
            {
                *p++ = '<';
                s += 3;
            }
            else if(size >= 3 && strncmp(s, "gt;", 3) == 0)
            {
                *p++ = '>';
                s += 3;
            }
            else if(size >= 4 && strncmp(s, "amp;", 4) == 0)
            {
                *p++ = '&';
                s += 4;
            }
            else if(size >= 5 && strncmp(s, "quot;", 5) == 0)
            {
                *p++ = '\"';
                s += 5;
            }
            else if(size >= 5 && strncmp(s, "apos;", 5) == 0)
            {
                *p++ = '\'';
                s += 5;
            }
            else
            {
                return false;
            }
        }
        else
        {
            *p++ = *s++;
        }
    }

    *p = '\0';

    return true;
}

/*--------------------------------------------------------------------------------------------------------------------*/

static void tokenizer_next(xml_parser_t *parser)
{
    /*----------------------------------------------------------------------------------------------------------------*/

    while(parser->size > 0 && isspace((unsigned char) *parser->buff))
    {
        parser->buff++;
        parser->size--;
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    if(parser->size == 0)
    {
        parser->curr_token.token_type = XML_TOKEN_EOF;

        return;
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    STR_t start = parser->buff;
    STR_t end = parser->buff;

    xml_token_type_t type;

    switch(*parser->buff)
    {
        /*------------------------------------------------------------------------------------------------------------*/

        case '\0':
            type = XML_TOKEN_EOF;
            break;

        /*------------------------------------------------------------------------------------------------------------*/

        case '<':
            /*--------------------------------------------------------------------------------------------------------*/

            /**/ if(parser->size >= 4 && strncmp(end, "<!--", 4) == 0)
            {
                end += 4;
                parser->size -= 4;
                while(parser->size >= 3 && strncmp(end, "-->", 3) != 0)
                {
                    if(*end == '\0')
                    {
                        type = XML_TOKEN_ERROR;
                        goto _bye;
                    }
                    end++;
                    parser->size--;
                }
                if(parser->size < 3)
                {
                    type = XML_TOKEN_ERROR;
                    goto _bye;
                }
                end += 3;
                parser->size -= 3;
                type = XML_TOKEN_COMMENT;
            }

            /*--------------------------------------------------------------------------------------------------------*/

            else if(parser->size >= 9 && strncmp(end, "<![CDATA[", 9) == 0)
            {
                end += 9;
                parser->size -= 9;
                while(parser->size >= 3 && strncmp(end, "]]>", 3) != 0)
                {
                    if(*end == '\0')
                    {
                        type = XML_TOKEN_ERROR;
                        goto _bye;
                    }
                    end++;
                    parser->size--;
                }
                if(parser->size < 3)
                {
                    type = XML_TOKEN_ERROR;
                    goto _bye;
                }
                end += 3;
                parser->size -= 3;
                type = XML_TOKEN_CDATA;
            }

            /*--------------------------------------------------------------------------------------------------------*/

            else
            {
                parser->tag = true;

                if(parser->size >= 2 && *(end + 1) == '/')
                {
                    end += 2;
                    parser->size -= 2;
                    type = XML_TOKEN_LT2;
                }
                else
                {
                    end += 1;
                    parser->size -= 1;
                    type = XML_TOKEN_LT1;
                }
            }

            /*--------------------------------------------------------------------------------------------------------*/
            break;

        /*------------------------------------------------------------------------------------------------------------*/

        case '>':
            parser->tag = false;

            end++;
            parser->size--;
            type = XML_TOKEN_GT;
            break;

        /*------------------------------------------------------------------------------------------------------------*/

        case '/':
            if(parser->tag == false) {
                goto  _text;
            }

            end++;
            parser->size--;
            type = XML_TOKEN_SLASH;
            break;

        /*------------------------------------------------------------------------------------------------------------*/

        case '=':
            if(parser->tag == false) {
                goto  _text;
            }

            end++;
            parser->size--;
            type = XML_TOKEN_EQUALS;
            break;

        /*------------------------------------------------------------------------------------------------------------*/

        case '\"':
            if(parser->tag == false) {
               goto  _text;
            }

            end++;
            parser->size--;
            while(parser->size >= 1 && *end != '\"')
            {
                if(*end == '\0')
                {
                    type = XML_TOKEN_ERROR;
                    goto _bye;
                }
                end++;
                parser->size--;
            }
            if(parser->size < 1)
            {
                type = XML_TOKEN_ERROR;
                goto _bye;
            }
            end++;
            parser->size--;
            type = XML_TOKEN_STRING;
            break;

        /*------------------------------------------------------------------------------------------------------------*/

        case '\'':
            if(parser->tag == false) {
                goto  _text;
            }

            end++;
            parser->size--;
            while(parser->size >= 1 && *end != '\'')
            {
                if(*end == '\0')
                {
                    type = XML_TOKEN_ERROR;
                    goto _bye;
                }
                end++;
                parser->size--;
            }
            if(parser->size < 1)
            {
                type = XML_TOKEN_ERROR;
                goto _bye;
            }
            end++;
            parser->size--;
            type = XML_TOKEN_STRING;
            break;

        /*------------------------------------------------------------------------------------------------------------*/

        default:
            if(parser->tag)
            {
                /*----------------------------------------------------------------------------------------------------*/

                while(parser->size >= 1 && XML_IDENT_TAB[(int) *end])
                {
                    if(*end == '\0')
                    {
                        type = XML_TOKEN_ERROR;
                        goto _bye;
                    }
                    end++;
                    parser->size--;
                }

                type = XML_TOKEN_IDENT;

                /*----------------------------------------------------------------------------------------------------*/
            }
            else
            {
                /*----------------------------------------------------------------------------------------------------*/
_text:
                while(parser->size >= 1 && *end != '<')
                {
                    if(*end == '\0')
                    {
                        type = XML_TOKEN_ERROR;
                        goto _bye;
                    }
                    end++;
                    parser->size--;
                }

                type = XML_TOKEN_TEXT;

                /*----------------------------------------------------------------------------------------------------*/
            }

            break;

        /*------------------------------------------------------------------------------------------------------------*/
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    /**/ if(type == XML_TOKEN_COMMENT) // <!-- ... -->
    {
        STR_t s = start + 4;
        STR_t e =  end  - 3;

        parser->curr_token.value = nyx_string_ndup(s, (size_t) e - (size_t) s);
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    else if(type == XML_TOKEN_CDATA) // <![CDATA[ ... ]]>
    {
        STR_t s = start + 9;
        STR_t e =  end  - 3;

        parser->curr_token.value = nyx_string_ndup(s, (size_t) e - (size_t) s);
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    else if(type == XML_TOKEN_IDENT)
    {
        STR_t s = start;
        STR_t e =  end ;

        size_t length = (size_t) e - (size_t) s;

        if(length > 0)
        {
            parser->curr_token.value = nyx_string_ndup(s, length);
        }
        else
        {
            type = XML_TOKEN_ERROR;
        }
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    else if(type == XML_TOKEN_STRING)
    {
        STR_t s = start + 1;
        STR_t e =  end  - 1;

        size_t length = (size_t) e - (size_t) s;

        str_t p = parser->curr_token.value = nyx_memory_alloc(length + 1);

        if(xmlcpy(p, s, e) == false)
        {
            nyx_memory_free(parser->curr_token.value);
            parser->curr_token.value = NULL;
            type = XML_TOKEN_ERROR;
            goto _bye;
        }
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    else if(type == XML_TOKEN_TEXT)
    {
        STR_t s = start;
        STR_t e =  end ;

        size_t length = (size_t) e - (size_t) s;

        if(length > 0)
        {
            str_t p = parser->curr_token.value = nyx_memory_alloc(length + 1);

            if(xmlcpy(p, s, e) == false)
            {
                nyx_memory_free(parser->curr_token.value);
                parser->curr_token.value = NULL;
                type = XML_TOKEN_ERROR;
                goto _bye;
            }
        }
        else
        {
            type = XML_TOKEN_ERROR;
        }
    }

    /*----------------------------------------------------------------------------------------------------------------*/

_bye:
    parser->curr_token.token_type = type;

    parser->buff = end;
}

/*--------------------------------------------------------------------------------------------------------------------*/
/* PARSER                                                                                                             */
/*--------------------------------------------------------------------------------------------------------------------*/

static nyx_xmldoc_t *xml_parse_content(xml_parser_t *parser, nyx_xmldoc_t *parent);

/*--------------------------------------------------------------------------------------------------------------------*/

static nyx_xmldoc_t *xml_parse_comment_node(xml_parser_t *parser, nyx_xmldoc_t *parent)
{
    /*----------------------------------------------------------------------------------------------------------------*/

    if(CHECK(XML_TOKEN_COMMENT) == false)
    {
        return NULL;
    }

    str_t data = PEEK().value;

    NEXT();

    /*----------------------------------------------------------------------------------------------------------------*/

    nyx_xmldoc_t *result = nyx_xmldoc_new(NYX_XML_COMMENT);
    result->data = data;
    result->parent = parent;

    return result;
}

/*--------------------------------------------------------------------------------------------------------------------*/

static nyx_xmldoc_t *xml_parse_cdata_node(xml_parser_t *parser, nyx_xmldoc_t *parent)
{
    /*----------------------------------------------------------------------------------------------------------------*/

    if(CHECK(XML_TOKEN_CDATA) == false)
    {
        return NULL;
    }

    str_t data = PEEK().value;

    NEXT();

    /*----------------------------------------------------------------------------------------------------------------*/

    nyx_xmldoc_t *result = nyx_xmldoc_new(NYX_XML_CDATA);
    result->data = data;
    result->parent = parent;

    return result;
}

/*--------------------------------------------------------------------------------------------------------------------*/

static nyx_xmldoc_t *xml_parse_text_node(xml_parser_t *parser, nyx_xmldoc_t *parent)
{
    /*----------------------------------------------------------------------------------------------------------------*/

    if(CHECK(XML_TOKEN_TEXT) == false)
    {
        return NULL;
    }

    str_t data = PEEK().value;

    NEXT();

    /*----------------------------------------------------------------------------------------------------------------*/

    nyx_xmldoc_t *result = nyx_xmldoc_new(NYX_XML_TEXT);
    result->data = data;
    result->parent = parent;

    return result;
}

/*--------------------------------------------------------------------------------------------------------------------*/

static nyx_xmldoc_t *xml_parse_attribute_node(xml_parser_t *parser)
{
    /*----------------------------------------------------------------------------------------------------------------*/

    if(CHECK(XML_TOKEN_IDENT) == false)
    {
        ///_memory_free(name);
        return NULL;
    }

    str_t name = PEEK().value;

    NEXT();

    /*----------------------------------------------------------------------------------------------------------------*/

    if(CHECK(XML_TOKEN_EQUALS) == false)
    {
        nyx_memory_free(name);
        return NULL;
    }

    NEXT();

    /*----------------------------------------------------------------------------------------------------------------*/

    if(CHECK(XML_TOKEN_STRING) == false)
    {
        nyx_memory_free(name);
        return NULL;
    }

    str_t data = PEEK().value;

    NEXT();

    /*----------------------------------------------------------------------------------------------------------------*/

    nyx_xmldoc_t *result = nyx_xmldoc_new(NYX_XML_ATTR);
    result->name = name;
    result->data = data;

    return result;
}

/*--------------------------------------------------------------------------------------------------------------------*/

static nyx_xmldoc_t *xml_parse_opening_tag(xml_parser_t *parser, nyx_xmldoc_t *parent)
{
    /*----------------------------------------------------------------------------------------------------------------*/

    if(CHECK(XML_TOKEN_LT1) == false)
    {
        return NULL;
    }

    NEXT();

    /*----------------------------------------------------------------------------------------------------------------*/

    if(CHECK(XML_TOKEN_IDENT) == false)
    {
        return NULL;
    }

    str_t name = PEEK().value;

    NEXT();

    /*----------------------------------------------------------------------------------------------------------------*/

    nyx_xmldoc_t *first_attr = NULL;
    nyx_xmldoc_t *last_attr = NULL;

    while(CHECK(XML_TOKEN_IDENT))
    {
        /*------------------------------------------------------------------------------------------------------------*/

        nyx_xmldoc_t *node = xml_parse_attribute_node(parser);

        if(node == NULL)
        {
            goto _err;
        }

        /*------------------------------------------------------------------------------------------------------------*/

        if(last_attr != NULL) {
            last_attr->next = node;
        } else {
            first_attr = node;
        }

        last_attr = node;

        /*------------------------------------------------------------------------------------------------------------*/
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    bool self_closing;

    if(CHECK(XML_TOKEN_SLASH) == false)
    {
        self_closing = false;

        ////();
    }
    else
    {
        self_closing = true;

        NEXT();
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    if(CHECK(XML_TOKEN_GT) == false)
    {
        goto _err;
    }

    NEXT();

    /*----------------------------------------------------------------------------------------------------------------*/

    nyx_xmldoc_t *result = nyx_xmldoc_new(NYX_XML_ELEM);
    result->name = name;
    result->parent = parent;
    result->attributes = first_attr;
    result->self_closing = self_closing;

    return result;

_err:
    while(first_attr != NULL)
    {
        nyx_xmldoc_t *tmp_attr = first_attr;
        first_attr = first_attr->next;
        nyx_xmldoc_free(tmp_attr);
    }

    nyx_memory_free(name);

    return NULL;
}

/*--------------------------------------------------------------------------------------------------------------------*/

static bool xml_parse_closing_tag(xml_parser_t *parser, const nyx_xmldoc_t *current)
{
    if(current->self_closing == false)
    {
        /*------------------------------------------------------------------------------------------------------------*/

        if(CHECK(XML_TOKEN_LT2) == false)
        {
            return false;
        }

        NEXT();

        /*------------------------------------------------------------------------------------------------------------*/

        if(CHECK(XML_TOKEN_IDENT) == false)
        {
            return false;
        }

        bool bad = strcmp(PEEK().value, current->name) != 0;

        nyx_memory_free(PEEK().value);

        if(bad) {
            return false;
        }
        else {
            NEXT();
        }

        /*------------------------------------------------------------------------------------------------------------*/

        if(CHECK(XML_TOKEN_GT) == false)
        {
            return false;
        }

        NEXT();

        /*------------------------------------------------------------------------------------------------------------*/
    }

    return true;
}

/*--------------------------------------------------------------------------------------------------------------------*/

static nyx_xmldoc_t *xml_parse_element_node(xml_parser_t *parser, nyx_xmldoc_t *parent) // NOLINT(*-no-recursion)
{
    /*----------------------------------------------------------------------------------------------------------------*/

    nyx_xmldoc_t *result = xml_parse_opening_tag(parser, parent);

    if(result == NULL)
    {
        nyx_xmldoc_free(result);
        return NULL;
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    result->children = xml_parse_content(parser, result);

    /*----------------------------------------------------------------------------------------------------------------*/

    bool okay = xml_parse_closing_tag(parser, result);

    if(okay == false)
    {
        nyx_xmldoc_free(result);
        return NULL;
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    return result;
}

/*--------------------------------------------------------------------------------------------------------------------*/

static nyx_xmldoc_t *xml_parse_content(xml_parser_t *parser, nyx_xmldoc_t *parent) // NOLINT(*-no-recursion)
{
    if(parent->self_closing)
    {
        return NULL;
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    nyx_xmldoc_t *first_child = NULL;
    nyx_xmldoc_t *last_child = NULL;

    for(nyx_xmldoc_t *node;;)
    {
        /*------------------------------------------------------------------------------------------------------------*/

        node = xml_parse_element_node(parser, parent);
        if(node == NULL)
        {
            node = xml_parse_comment_node(parser, parent);
            if(node == NULL)
            {
                node = xml_parse_cdata_node(parser, parent);
                if(node == NULL)
                {
                    node = xml_parse_text_node(parser, parent);
                    if(node == NULL)
                    {
                        break;
                    }
                }
            }
        }

        /*------------------------------------------------------------------------------------------------------------*/

        if(last_child != NULL) {
            last_child->next = node;
        } else {
            first_child = node;
        }

        last_child = node;

        /*------------------------------------------------------------------------------------------------------------*/

        node->parent = parent;

        /*------------------------------------------------------------------------------------------------------------*/
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    return first_child;
}

/*--------------------------------------------------------------------------------------------------------------------*/

nyx_xmldoc_t *nyx_xmldoc_parse_buff(size_t size, BUFF_t buff)
{
    if(size == 0x00
       ||
       buff == NULL
    ) {
        return NULL;
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    xml_parser_t *parser = &(xml_parser_t) {
        .tag = false,
        .size = size,
        .buff = buff,
        .curr_token = {
            .value = NULL,
            .token_type = XML_TOKEN_ERROR,
        },
    };

    /*----------------------------------------------------------------------------------------------------------------*/

    NEXT();

    /*----------------------------------------------------------------------------------------------------------------*/

	nyx_xmldoc_t *result = xml_parse_element_node(parser, NULL);

    /*----------------------------------------------------------------------------------------------------------------*/

    if(result != NULL && CHECK(XML_TOKEN_EOF) == false)
    {
        nyx_xmldoc_free(result);

        result = NULL;
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    return result;
}

/*--------------------------------------------------------------------------------------------------------------------*/

nyx_xmldoc_t *nyx_xmldoc_parse(STR_t string)
{
    return nyx_xmldoc_parse_buff(
        strlen(string),
        /*--*/(string)
    );
}

/*--------------------------------------------------------------------------------------------------------------------*/
#endif
/*--------------------------------------------------------------------------------------------------------------------*/
