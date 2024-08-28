/*--------------------------------------------------------------------------------------------------------------------*/

#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCUnusedMacroInspection"
#pragma ide diagnostic ignored "bugprone-reserved-identifier"
#pragma ide diagnostic ignored "OCUnusedGlobalDeclarationInspection"

/*--------------------------------------------------------------------------------------------------------------------*/

#ifndef NYX_BASE_NYX_BASE_INTERNAL_H
#define NYX_BASE_NYX_BASE_INTERNAL_H

/*--------------------------------------------------------------------------------------------------------------------*/

#include "nyx_node.h"

/*--------------------------------------------------------------------------------------------------------------------*/

#ifdef __cplusplus
extern "C" {
#endif

/*--------------------------------------------------------------------------------------------------------------------*/

#define __UNUSED__ \
            __attribute__((unused))

/*--------------------------------------------------------------------------------------------------------------------*/
/* OBJECT                                                                                                             */
/*--------------------------------------------------------------------------------------------------------------------*/

void nyx_object_notify(
    nyx_object_t *object,
    bool modified
);

/*--------------------------------------------------------------------------------------------------------------------*/
/* MEMORY                                                                                                             */
/*--------------------------------------------------------------------------------------------------------------------*/

str_t nyx_boolean_dup(
    bool b
);

str_t nyx_double_dup(
    double d
);

str_t nyx_string_dup(
    STR_t s
);

/*--------------------------------------------------------------------------------------------------------------------*/
/* UTF-8                                                                                                              */
/*--------------------------------------------------------------------------------------------------------------------*/

int nyx_unicode_to_utf8(
    uint32_t unicode_char,
    str_t p
);

/*--------------------------------------------------------------------------------------------------------------------*/
/* STRING BUILDER                                                                                                     */
/*--------------------------------------------------------------------------------------------------------------------*/

typedef struct
{
    struct nyx_string_builder_node_s *head;
    struct nyx_string_builder_node_s *tail;

} nyx_string_builder_t;

/*--------------------------------------------------------------------------------------------------------------------*/

nyx_string_builder_t *nyx_string_builder_new();

void nyx_string_builder_free(
    nyx_string_builder_t *sb
);

void nyx_string_builder_clear(
    nyx_string_builder_t *sb
);

void nyx_string_builder_append_n(
    nyx_string_builder_t *sb,
    STR_t args[],
    size_t n
);

size_t nyx_string_builder_length(
    nyx_string_builder_t *sb
);

str_t nyx_string_builder_to_string(
    nyx_string_builder_t *sb
);

str_t nyx_string_builder_to_cstring(
    nyx_string_builder_t *sb
);

/*--------------------------------------------------------------------------------------------------------------------*/

#define nyx_string_builder_append(sb, ...) ({                                                                          \
                                                                                                                       \
    STR_t args[] = {__VA_ARGS__};                                                                                      \
                                                                                                                       \
    nyx_string_builder_append_n(sb, args, sizeof(args) / sizeof(STR_t));                                               \
})

/*--------------------------------------------------------------------------------------------------------------------*/

#define nyx_string_builder_from(...) ({                                                                                \
                                                                                                                       \
    STR_t args[] = {__VA_ARGS__};                                                                                      \
                                                                                                                       \
    nyx_string_builder_t *_sb = nyx_string_builder_new();                                                              \
                                                                                                                       \
    nyx_string_builder_append_n(_sb, args, sizeof(args) / sizeof(STR_t));                                              \
                                                                                                                       \
    _sb;                                                                                                               \
})

/*--------------------------------------------------------------------------------------------------------------------*/
/* HELPERS                                                                                                            */
/*--------------------------------------------------------------------------------------------------------------------*/

void internal_get_timestamp(str_t timestamp_buff, size_t timestamp_size);

void internal_set_opts(nyx_dict_t *dict, nyx_opts_t *opts);

bool internal_copy_entry(nyx_dict_t *dst, const nyx_dict_t *src, STR_t key);

nyx_dict_t *internal_xxx_set_vector_new(const nyx_dict_t *def_vector, STR_t set_tagname, STR_t one_tagname);

/*--------------------------------------------------------------------------------------------------------------------*/
/* STREAM                                                                                                             */
/*--------------------------------------------------------------------------------------------------------------------*/

typedef struct
{
    int idx;

    STR_t s_ptr;
    STR_t e_ptr;

    size_t pos;
    size_t len;

} nyx_stream_t;

/*--------------------------------------------------------------------------------------------------------------------*/

#define NYX_STREAM() \
            ((nyx_stream_t) {.idx = 0x00, .s_ptr = NULL, .e_ptr = NULL, .pos = 0, .len = 0})

/*--------------------------------------------------------------------------------------------------------------------*/

bool nyx_stream_detect_opening_tag(
        nyx_stream_t *stream,
        size_t size,
        BUFF_t buff
);

bool nyx_stream_detect_closing_tag(
        nyx_stream_t *stream,
        size_t size,
        BUFF_t buff
);

/*--------------------------------------------------------------------------------------------------------------------*/

#ifdef __cplusplus
}
#endif

/*--------------------------------------------------------------------------------------------------------------------*/

#endif /* NYX_BASE_NYX_BASE_INTERNAL_H */

/*--------------------------------------------------------------------------------------------------------------------*/

#pragma clang diagnostic pop

/*--------------------------------------------------------------------------------------------------------------------*/
