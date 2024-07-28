/*--------------------------------------------------------------------------------------------------------------------*/

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "indi_node_internal.h"

/*--------------------------------------------------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------------------------------------------------*/

typedef enum json_token_type_e
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

typedef struct json_token_s
{
    str_t value;

    json_token_type_t token_type;

} json_token_t;

/*--------------------------------------------------------------------------------------------------------------------*/

typedef struct json_parser_s
{
    STR_t pos;

    json_token_t curr_token;

} json_parser_t;

/*--------------------------------------------------------------------------------------------------------------------*/
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
            if(CHECK(JSON_TOKEN_NUMBER) || CHECK(JSON_TOKEN_STRING))        \
            {                                                               \
                indi_memory_free(PEEK().value);                             \
                                                                            \
                PEEK().value = NULL;                                        \
            }                                                               \
                                                                            \
            PEEK().token_type = (t)                                         \

/*--------------------------------------------------------------------------------------------------------------------*/

#define TRIM(s, e) \
    ({                                                                      \
        size_t result = (size_t) (e) - (size_t) (s);                        \
                                                                            \
        while((isspace(*((s) + 0)) || *((s) + 0) == '"') && result > 0) {   \
            (s)++;                                                          \
            result--;                                                       \
        }                                                                   \
                                                                            \
        while((isspace(*((e) - 1)) || *((e) - 1) == '"') && result > 0) {   \
            (e)--;                                                          \
            result--;                                                       \
        }                                                                   \
                                                                            \
        result;                                                             \
    })

/*--------------------------------------------------------------------------------------------------------------------*/

int indi_unicode_to_utf8(uint32_t unicode_char, str_t p)
{
    unsigned char *up = (unsigned char *) p;

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

static void tokenizer_next(json_parser_t *parser)
{
    /*----------------------------------------------------------------------------------------------------------------*/

    for(; isspace(*parser->pos); parser->pos++);

    /*----------------------------------------------------------------------------------------------------------------*/

    STR_t start = parser->pos;
    STR_t end = parser->pos;

    json_token_type_t type;

    switch(*parser->pos)
    {
        case '\0':
            type = JSON_TOKEN_EOF;
            break;

        case '{':
            type = JSON_TOKEN_CURLY_OPEN;
            end++;
            break;

        case '}':
            type = JSON_TOKEN_CURLY_CLOSE;
            end++;
            break;

        case '[':
            type = JSON_TOKEN_SQUARE_OPEN;
            end++;
            break;

        case ']':
            type = JSON_TOKEN_SQUARE_CLOSE;
            end++;
            break;

        case ':':
            type = JSON_TOKEN_COLON;
            end++;
            break;

        case ',':
            type = JSON_TOKEN_COMMA;
            end++;
            break;

        case '"':
            type = JSON_TOKEN_STRING;
            end++;
            while(*end != '\"')
            {
                if(*end == '\0')
                {
                    type = JSON_TOKEN_ERROR;
                    goto _bye;
                }

                if(*(end + 0) == '\\'
                   &&
                   *(end + 1) != '\0'
                ) {
                    end++;
                }

                end++;
            }
            end++;
            break;

        default:
            /**/ if(*end == '-' || isdigit(*end))
            {
                type = JSON_TOKEN_NUMBER;
                end++;
                while(*end == '-' || *end == '+' || *end == '.' || *end == 'e' || *end == 'E' || isdigit(*end))
                {
                    end++;
                }
            }
            else if(strncmp(end, "null", 4) == 0)
            {
                type = JSON_TOKEN_NULL;
                end += 4;
            }
            else if(strncmp(end, "true", 4) == 0)
            {
                type = JSON_TOKEN_TRUE;
                end += 4;
            }
            else if(strncmp(end, "false", 5) == 0)
            {
                type = JSON_TOKEN_FALSE;
                end += 5;
            }
            else
            {
                type = JSON_TOKEN_ERROR;
            }
            break;
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    /**/ if(type == JSON_TOKEN_NUMBER)
    {
        STR_t s = start;
        STR_t e =  end ;

        size_t length = TRIM(s, e);

        str_t p = parser->curr_token.value = indi_memory_alloc(length + 1);

        /* COPY VALUE */

        strncpy(p, s, length)[length] = '\0';
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    else if(type == JSON_TOKEN_STRING)
    {
        STR_t s = start;
        STR_t e =  end ;

        size_t length = TRIM(s, e);

        str_t p = parser->curr_token.value = indi_memory_alloc(length + 1);

        /* COPY VALUE */

        while(s < e)
        {
            if(*s == '\\')
            {
                s++;
                if(e - s < 1)
                {
                    indi_memory_free(parser->curr_token.value);
                    parser->curr_token.value = NULL;
                    type = JSON_TOKEN_ERROR;
                    goto _bye;
                }
                else
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
                            if(e - s < 4)
                            {
                                indi_memory_free(parser->curr_token.value);
                                parser->curr_token.value = NULL;
                                type = JSON_TOKEN_ERROR;
                                goto _bye;
                            }
                            else
                            {
                                char hex[5] = {
                                    s[0], s[1],
                                    s[2], s[3],
                                    '\0',
                                };

                                uint32_t unicode_char = (uint32_t) strtol(hex, NULL, 16);

                                s += 0x00000000000000000000000000000000004 - 1;
                                p += indi_unicode_to_utf8(unicode_char, p) - 1;
                            }
                            break;
                        default:
                            *p = *s;
                            break;
                    }
                }
            }
            else
            {
                *p = *s;
                /*--*/
            }

            s++;
            p++;
        }

        *p = '\0';
    }

    /*----------------------------------------------------------------------------------------------------------------*/

_bye:
    parser->curr_token.token_type = type;

    parser->pos = end;
}

/*--------------------------------------------------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------------------------------------------------*/

static indi_dict_t *json_parse_dict(json_parser_t *parser);

static indi_list_t *json_parse_list(json_parser_t *parser);

/*--------------------------------------------------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------------------------------------------------*/

static indi_null_t *json_parse_null(json_parser_t *parser)
{
    if(CHECK(JSON_TOKEN_NULL) == false)
    {
        return NULL;
    }

    NEXT();

    return indi_null_new();
}

/*--------------------------------------------------------------------------------------------------------------------*/

static indi_boolean_t *json_parse_true(json_parser_t *parser)
{
    if(CHECK(JSON_TOKEN_TRUE) == false)
    {
        return NULL;
    }

    NEXT();

    return indi_boolean_from(true);
}

/*--------------------------------------------------------------------------------------------------------------------*/

static indi_boolean_t *json_parse_false(json_parser_t *parser)
{
    if(CHECK(JSON_TOKEN_FALSE) == false)
    {
        return NULL;
    }

    NEXT();

    return indi_boolean_from(false);
}

/*--------------------------------------------------------------------------------------------------------------------*/

static indi_number_t *json_parse_number(json_parser_t *parser)
{
    if(CHECK(JSON_TOKEN_NUMBER == false))
    {
        return NULL;
    }

    str_t value = PEEK().value;

    indi_number_t *result = indi_number_from(atof(value)); // NOLINT(*-err34-c)

    indi_memory_free(value);

    NEXT();

    return result;
}

/*--------------------------------------------------------------------------------------------------------------------*/

static indi_string_t *json_parse_string(json_parser_t *parser)
{
    if(CHECK(JSON_TOKEN_STRING) == false)
    {
        return NULL;
    }

    str_t value = PEEK().value;

    indi_string_t *result = indi_string_from(/**/(value)); // NOLINT(*-err34-c)

    indi_memory_free(value);

    NEXT();

    return result;
}

/*--------------------------------------------------------------------------------------------------------------------*/

static indi_dict_t *json_parse_dict(json_parser_t *parser) // NOLINT(misc-no-recursion)
{
    indi_dict_t *result = indi_dict_new();

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
            indi_memory_free(key);

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
            indi_memory_free(key);

            goto _err;
        }

        /*------------------------------------------------------------------------------------------------------------*/

        indi_dict_set(result, key, value);

        /*------------------------------------------------------------------------------------------------------------*/

        indi_memory_free(key);

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

    indi_dict_free(result);

    return NULL;
}

/*--------------------------------------------------------------------------------------------------------------------*/

static indi_list_t *json_parse_list(json_parser_t *parser) // NOLINT(misc-no-recursion)
{
    indi_list_t *result = indi_list_new();

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

        indi_list_push(result, value);

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

    indi_list_free(result);

    return NULL;
}

/*--------------------------------------------------------------------------------------------------------------------*/

indi_object_t *indi_object_parse(__NULLABLE__ STR_t text)
{
    if(text == NULL)
    {
        return NULL;
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    json_parser_t *parser = &((json_parser_t) {
        .pos = text,
        .curr_token = {
            .value = NULL,
            .token_type = JSON_TOKEN_ERROR
        }
    });

    /*----------------------------------------------------------------------------------------------------------------*/

    NEXT();

    /*----------------------------------------------------------------------------------------------------------------*/

    indi_object_t *result = NULL;

    /**/ if(CHECK(JSON_TOKEN_NULL)) {
        result = (indi_object_t *) json_parse_null(parser);
    }
    else if(CHECK(JSON_TOKEN_TRUE)) {
        result = (indi_object_t *) json_parse_true(parser);
    }
    else if(CHECK(JSON_TOKEN_FALSE)) {
        result = (indi_object_t *) json_parse_false(parser);
    }
    else if(CHECK(JSON_TOKEN_NUMBER)) {
        result = (indi_object_t *) json_parse_number(parser);
    }
    else if(CHECK(JSON_TOKEN_STRING)) {
        result = (indi_object_t *) json_parse_string(parser);
    }
    else if(CHECK(JSON_TOKEN_CURLY_OPEN)) {
        result = (indi_object_t *) json_parse_dict(parser);
    }
    else if(CHECK(JSON_TOKEN_SQUARE_OPEN)) {
        result = (indi_object_t *) json_parse_list(parser);
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    if(result != NULL && CHECK(JSON_TOKEN_EOF) == false)
    {
        indi_object_free(result);

        return NULL;
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    return result;
}

/*--------------------------------------------------------------------------------------------------------------------*/
