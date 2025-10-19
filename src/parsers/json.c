/* NyxNode
 * Author: Jérôme ODIER <jerome.odier@lpsc.in2p3.fr>
 * SPDX-License-Identifier: GPL-2.0-only (Mongoose backend) or GPL-3.0+
 */

/*--------------------------------------------------------------------------------------------------------------------*/

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "../nyx_node_internal.h"

/*--------------------------------------------------------------------------------------------------------------------*/
/* DEFINITIONS                                                                                                        */
/*--------------------------------------------------------------------------------------------------------------------*/

typedef enum
{
    JSON_TOKEN_EOF,
    JSON_TOKEN_NULL,
    JSON_TOKEN_TRUE,
    JSON_TOKEN_FALSE,
    JSON_TOKEN_NUMBER,
    JSON_TOKEN_STRING,
    JSON_TOKEN_CURLY_OPEN,
    JSON_TOKEN_CURLY_CLOSE,
    JSON_TOKEN_SQUARE_OPEN,
    JSON_TOKEN_SQUARE_CLOSE,
    JSON_TOKEN_COLON,
    JSON_TOKEN_COMMA,
    JSON_TOKEN_ERROR,

} json_token_type_t;

/*--------------------------------------------------------------------------------------------------------------------*/

typedef struct
{
    str_t value;

    json_token_type_t token_type;

} json_token_t;

/*--------------------------------------------------------------------------------------------------------------------*/

typedef struct
{
    size_t size;
    STR_t buff;

    json_token_t curr_token;

} json_parser_t;

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

#define RELEASE(t) \
            if(CHECK(JSON_TOKEN_NUMBER) || CHECK(JSON_TOKEN_STRING))                        \
            {                                                                               \
                nyx_memory_free(PEEK().value);                                              \
                                                                                            \
                PEEK().value = NULL;                                                        \
            }                                                                               \
                                                                                            \
            PEEK().token_type = (t)                                                         \

/*--------------------------------------------------------------------------------------------------------------------*/

#define TRIM(s, e) \
    ({                                                                                      \
        size_t result = (size_t) (e) - (size_t) (s);                                        \
                                                                                            \
        while((isspace((unsigned char) *((s) + 0)) || *((s) + 0) == '"') && result > 0) {   \
            (s)++;                                                                          \
            result--;                                                                       \
        }                                                                                   \
                                                                                            \
        while((isspace((unsigned char) *((e) - 1)) || *((e) - 1) == '"') && result > 0) {   \
            (e)--;                                                                          \
            result--;                                                                       \
        }                                                                                   \
                                                                                            \
        result;                                                                             \
    })

/*--------------------------------------------------------------------------------------------------------------------*/

static bool jsoncpy(str_t p, STR_t s, STR_t e)
{
    while(s < e)
    {
        if(*s == '\\')
        {
            s++;

            if(e - s >= 1)
            {
                switch(*s)
                {
                    case '\"': *p = '\"'; break;
                    case '\\': *p = '\\'; break;
                    case '/': *p = '/'; break;
                    case 'b': *p = '\b'; break;
                    case 'f': *p = '\f'; break;
                    case 'n': *p = '\n'; break;
                    case 'r': *p = '\r'; break;
                    case 't': *p = '\t'; break;
                    case 'u':
                        s++;

                        if(e - s >= 4)
                        {
                            char hex[5] = {
                                s[0], s[1],
                                s[2], s[3],
                                '\0',
                            };

                            uint32_t unicode_char = (uint32_t) strtol(hex, NULL, 16);

                            s += 0x0000000000000000000000000000000004;
                            p += nyx_unicode_to_utf8(p, unicode_char);
                        }
                        else
                        {
                            return false;
                        }
                        break;
                    default:
                        *p++ = *s++;
                        break;
                }
            }
            else
            {
                return false;
            }
        }
        else
        {
            *p++ = *s++;
            /*--*/
        }
    }

    *p = '\0';

    return true;
}

/*--------------------------------------------------------------------------------------------------------------------*/

static void tokenizer_next(json_parser_t *parser)
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
        parser->curr_token.token_type = JSON_TOKEN_EOF;

        return;
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    STR_t start = parser->buff;
    STR_t end = parser->buff;

    json_token_type_t type;

    switch(*parser->buff)
    {
        /*------------------------------------------------------------------------------------------------------------*/

        case '\0':
            type = JSON_TOKEN_EOF;
            break;

        /*------------------------------------------------------------------------------------------------------------*/

        case '{':
            end++;
            parser->size--;
            type = JSON_TOKEN_CURLY_OPEN;
            break;

        /*------------------------------------------------------------------------------------------------------------*/

        case '}':
            end++;
            parser->size--;
            type = JSON_TOKEN_CURLY_CLOSE;
            break;

        /*------------------------------------------------------------------------------------------------------------*/

        case '[':
            end++;
            parser->size--;
            type = JSON_TOKEN_SQUARE_OPEN;
            break;

        /*------------------------------------------------------------------------------------------------------------*/

        case ']':
            end++;
            parser->size--;
            type = JSON_TOKEN_SQUARE_CLOSE;
            break;

        /*------------------------------------------------------------------------------------------------------------*/

        case ':':
            end++;
            parser->size--;
            type = JSON_TOKEN_COLON;
            break;

        /*------------------------------------------------------------------------------------------------------------*/

        case ',':
            end++;
            parser->size--;
            type = JSON_TOKEN_COMMA;
            break;

        /*------------------------------------------------------------------------------------------------------------*/

        case '"':
            end++;
            parser->size--;
            while(parser->size >= 1 && *end != '"')
            {
                if(*end == '\0')
                {
                    type = JSON_TOKEN_ERROR;
                    goto _bye;
                }

                if(parser->size >= 2 && (
                   *(end + 0) == '\\'
                   &&
                   *(end + 1) != '\0'
                )) {
                    end++;
                    parser->size--;
                }

                end++;
                parser->size--;
            }
            if(parser->size < 1)
            {
                type = JSON_TOKEN_ERROR;
                goto _bye;
            }
            end++;
            parser->size--;
            type = JSON_TOKEN_STRING;
            break;

        /*------------------------------------------------------------------------------------------------------------*/

        default:
            /**/ if(*end == '-' || isdigit((unsigned char) *end))
            {
                end++;
                parser->size--;
                while(parser->size >= 1 && (*end == '-' || *end == '+' || *end == '.' || *end == 'e' || *end == 'E' || isdigit((unsigned char) *end)))
                {
                    end++;
                    parser->size--;
                }
                type = JSON_TOKEN_NUMBER;
            }
            else if(parser->size >= 4 && strncmp(end, "null", 4) == 0)
            {
                end += 4;
                parser->size -= 4;
                type = JSON_TOKEN_NULL;
            }
            else if(parser->size >= 4 && strncmp(end, "true", 4) == 0)
            {
                end += 4;
                parser->size -= 4;
                type = JSON_TOKEN_TRUE;
            }
            else if(parser->size >= 5 && strncmp(end, "false", 5) == 0)
            {
                end += 5;
                parser->size -= 5;
                type = JSON_TOKEN_FALSE;
            }
            else
            {
                type = JSON_TOKEN_ERROR;
            }
            break;

        /*------------------------------------------------------------------------------------------------------------*/
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    /**/ if(type == JSON_TOKEN_NUMBER)
    {
        STR_t s = start;
        STR_t e =  end ;

        size_t length = TRIM(s, e);

        parser->curr_token.value = nyx_string_ndup(s, length);
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    else if(type == JSON_TOKEN_STRING)
    {
        STR_t s = start;
        STR_t e =  end ;

        size_t length = TRIM(s, e);

        str_t p = parser->curr_token.value = nyx_memory_alloc(length + 1);

        if(jsoncpy(p, s, e) == false)
        {
            nyx_memory_free(parser->curr_token.value);
            parser->curr_token.value = NULL;
            type = JSON_TOKEN_ERROR;
            goto _bye;
        }
    }

    /*----------------------------------------------------------------------------------------------------------------*/

_bye:
    parser->curr_token.token_type = type;

    parser->buff = end;
}

/*--------------------------------------------------------------------------------------------------------------------*/
/* TOKENIZER                                                                                                          */
/*--------------------------------------------------------------------------------------------------------------------*/

static nyx_dict_t *json_parse_dict(json_parser_t *parser);

static nyx_list_t *json_parse_list(json_parser_t *parser);

/*--------------------------------------------------------------------------------------------------------------------*/

static nyx_null_t *json_parse_null(json_parser_t *parser)
{
    if(CHECK(JSON_TOKEN_NULL) == false)
    {
        return NULL;
    }

    NEXT();

    return nyx_null_new();
}

/*--------------------------------------------------------------------------------------------------------------------*/

static nyx_boolean_t *json_parse_true(json_parser_t *parser)
{
    if(CHECK(JSON_TOKEN_TRUE) == false)
    {
        return NULL;
    }

    NEXT();

    return nyx_boolean_from(true);
}

/*--------------------------------------------------------------------------------------------------------------------*/

static nyx_boolean_t *json_parse_false(json_parser_t *parser)
{
    if(CHECK(JSON_TOKEN_FALSE) == false)
    {
        return NULL;
    }

    NEXT();

    return nyx_boolean_from(false);
}

/*--------------------------------------------------------------------------------------------------------------------*/

static nyx_number_t *json_parse_number(json_parser_t *parser)
{
    if(CHECK(JSON_TOKEN_NUMBER == false))
    {
        return NULL;
    }

    str_t value = PEEK().value;

    nyx_number_t *result = nyx_number_from(atof(value)); // NOLINT(*-err34-c)

    nyx_memory_free(value);

    NEXT();

    return result;
}

/*--------------------------------------------------------------------------------------------------------------------*/

static nyx_string_t *json_parse_string(json_parser_t *parser)
{
    if(CHECK(JSON_TOKEN_STRING) == false)
    {
        return NULL;
    }

    str_t value = PEEK().value;

    nyx_string_t *result = nyx_string_from(/**/(value), true); // NOLINT(*-err34-c)

    // don't free de string

    NEXT();

    return result;
}

/*--------------------------------------------------------------------------------------------------------------------*/

static nyx_dict_t *json_parse_dict(json_parser_t *parser) // NOLINT(misc-no-recursion)
{
    nyx_dict_t *result = nyx_dict_new();

    /*----------------------------------------------------------------------------------------------------------------*/

    if(CHECK(JSON_TOKEN_CURLY_OPEN) == false)
    {
        goto _err;
    }

    NEXT();

    /*----------------------------------------------------------------------------------------------------------------*/

    while(CHECK(JSON_TOKEN_CURLY_CLOSE) == false)
    {
        /*------------------------------------------------------------------------------------------------------------*/

        if(CHECK(JSON_TOKEN_STRING) == false)
        {
            ////_memory_free(key);

            goto _err;
        }

        str_t key = PEEK().value;

        NEXT();

        if(CHECK(JSON_TOKEN_COLON) == false)
        {
            nyx_memory_free(key);

            goto _err;
        }

        NEXT();

        /*------------------------------------------------------------------------------------------------------------*/

        buff_t value = NULL;

        /**/ if(CHECK(JSON_TOKEN_NULL)) {
            value = json_parse_null(parser);
        }
        else if(CHECK(JSON_TOKEN_TRUE)) {
            value = json_parse_true(parser);
        }
        else if(CHECK(JSON_TOKEN_FALSE)) {
            value = json_parse_false(parser);
        }
        else if(CHECK(JSON_TOKEN_NUMBER)) {
            value = json_parse_number(parser);
        }
        else if(CHECK(JSON_TOKEN_STRING)) {
            value = json_parse_string(parser);
        }
        else if(CHECK(JSON_TOKEN_CURLY_OPEN)) {
            value = json_parse_dict(parser);
        }
        else if(CHECK(JSON_TOKEN_SQUARE_OPEN)) {
            value = json_parse_list(parser);
        }

        if(value == NULL)
        {
            nyx_memory_free(key);

            goto _err;
        }

        /*------------------------------------------------------------------------------------------------------------*/

        nyx_dict_set(result, key, value);

        /*------------------------------------------------------------------------------------------------------------*/

        nyx_memory_free(key);

        /*------------------------------------------------------------------------------------------------------------*/

        if(CHECK(JSON_TOKEN_COMMA))
        {
            NEXT();

            if(CHECK(JSON_TOKEN_CURLY_CLOSE) == true)
            {
                goto _err;
            }
        }
        else
        {
            if(CHECK(JSON_TOKEN_CURLY_CLOSE) == false)
            {
                goto _err;
            }
        }

        /*------------------------------------------------------------------------------------------------------------*/
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    if(CHECK(JSON_TOKEN_CURLY_CLOSE) == false)
    {
        goto _err;
    }

    NEXT();

    /*----------------------------------------------------------------------------------------------------------------*/

    return result;

_err:
    RELEASE(JSON_TOKEN_ERROR);

    nyx_dict_free(result);

    return NULL;
}

/*--------------------------------------------------------------------------------------------------------------------*/

static nyx_list_t *json_parse_list(json_parser_t *parser) // NOLINT(misc-no-recursion)
{
    nyx_list_t *result = nyx_list_new();

    /*----------------------------------------------------------------------------------------------------------------*/

    if(CHECK(JSON_TOKEN_SQUARE_OPEN) == false)
    {
        goto _err;
    }

    NEXT();

    /*----------------------------------------------------------------------------------------------------------------*/

    while(CHECK(JSON_TOKEN_SQUARE_CLOSE) == false)
    {
        /*------------------------------------------------------------------------------------------------------------*/

        buff_t value = NULL;

        /**/ if(CHECK(JSON_TOKEN_NULL)) {
            value = json_parse_null(parser);
        }
        else if(CHECK(JSON_TOKEN_TRUE)) {
            value = json_parse_true(parser);
        }
        else if(CHECK(JSON_TOKEN_FALSE)) {
            value = json_parse_false(parser);
        }
        else if(CHECK(JSON_TOKEN_NUMBER)) {
            value = json_parse_number(parser);
        }
        else if(CHECK(JSON_TOKEN_STRING)) {
            value = json_parse_string(parser);
        }
        else if(CHECK(JSON_TOKEN_CURLY_OPEN)) {
            value = json_parse_dict(parser);
        }
        else if(CHECK(JSON_TOKEN_SQUARE_OPEN)) {
            value = json_parse_list(parser);
        }

        if(value == NULL)
        {
            goto _err;
        }

        /*------------------------------------------------------------------------------------------------------------*/

        nyx_list_push(result, value);

        /*------------------------------------------------------------------------------------------------------------*/

        if(CHECK(JSON_TOKEN_COMMA))
        {
            NEXT();

            if(CHECK(JSON_TOKEN_SQUARE_CLOSE) == true)
            {
                goto _err;
            }
        }
        else
        {
            if(CHECK(JSON_TOKEN_SQUARE_CLOSE) == false)
            {
                goto _err;
            }
        }

        /*------------------------------------------------------------------------------------------------------------*/
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    if(CHECK(JSON_TOKEN_SQUARE_CLOSE) == false)
    {
        goto _err;
    }

    NEXT();

    /*----------------------------------------------------------------------------------------------------------------*/

    return result;

_err:
    RELEASE(JSON_TOKEN_ERROR);

    nyx_list_free(result);

    return NULL;
}

/*--------------------------------------------------------------------------------------------------------------------*/

__NULLABLE__ nyx_object_t *nyx_object_parse_buff(__ZEROABLE__ size_t size, __NULLABLE__ BUFF_t buff)
{
    if(size == 0x00
       ||
       buff == NULL
    ) {
        return NULL;
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    json_parser_t *parser = &((json_parser_t) {
        .size = size,
        .buff = buff,
        .curr_token = {
            .value = NULL,
            .token_type = JSON_TOKEN_ERROR,
        },
    });

    /*----------------------------------------------------------------------------------------------------------------*/

    NEXT();

    /*----------------------------------------------------------------------------------------------------------------*/

    nyx_object_t *result = NULL;

    /**/ if(CHECK(JSON_TOKEN_NULL)) {
        result = (nyx_object_t *) json_parse_null(parser);
    }
    else if(CHECK(JSON_TOKEN_TRUE)) {
        result = (nyx_object_t *) json_parse_true(parser);
    }
    else if(CHECK(JSON_TOKEN_FALSE)) {
        result = (nyx_object_t *) json_parse_false(parser);
    }
    else if(CHECK(JSON_TOKEN_NUMBER)) {
        result = (nyx_object_t *) json_parse_number(parser);
    }
    else if(CHECK(JSON_TOKEN_STRING)) {
        result = (nyx_object_t *) json_parse_string(parser);
    }
    else if(CHECK(JSON_TOKEN_CURLY_OPEN)) {
        result = (nyx_object_t *) json_parse_dict(parser);
    }
    else if(CHECK(JSON_TOKEN_SQUARE_OPEN)) {
        result = (nyx_object_t *) json_parse_list(parser);
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    if(result != NULL && CHECK(JSON_TOKEN_EOF) == false)
    {
        nyx_object_free(result);

        result = NULL;
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    return result;
}

/*--------------------------------------------------------------------------------------------------------------------*/

__NULLABLE__ nyx_object_t *nyx_object_parse(__NULLABLE__ STR_t string)
{
    return nyx_object_parse_buff(
        strlen(string),
        /*--*/(string)
    );
}

/*--------------------------------------------------------------------------------------------------------------------*/
