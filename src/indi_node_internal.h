/*--------------------------------------------------------------------------------------------------------------------*/

#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCUnusedMacroInspection"
#pragma ide diagnostic ignored "bugprone-reserved-identifier"
#pragma ide diagnostic ignored "OCUnusedGlobalDeclarationInspection"

/*--------------------------------------------------------------------------------------------------------------------*/

#ifndef INDI_BASE_INDI_BASE_INTERNAL_H
#define INDI_BASE_INDI_BASE_INTERNAL_H

/*--------------------------------------------------------------------------------------------------------------------*/

#include "indi_node.h"

/*--------------------------------------------------------------------------------------------------------------------*/

#ifdef __cplusplus
extern "C" {
#endif

/*--------------------------------------------------------------------------------------------------------------------*/

#define __USED__ __attribute__ ((unused))

/*--------------------------------------------------------------------------------------------------------------------*/
/* OBJECT                                                                                                             */
/*--------------------------------------------------------------------------------------------------------------------*/

void indi_object_notify(
    indi_object_t *object
);

/*--------------------------------------------------------------------------------------------------------------------*/
/* MEMORY                                                                                                             */
/*--------------------------------------------------------------------------------------------------------------------*/

str_t indi_boolean_dup(
    bool b
);

str_t indi_double_dup(
    double d
);

str_t indi_string_dup(
    STR_t s
);

/*--------------------------------------------------------------------------------------------------------------------*/
/* UTF-8                                                                                                              */
/*--------------------------------------------------------------------------------------------------------------------*/

int indi_unicode_to_utf8(
    uint32_t unicode_char,
    str_t p
);

/*--------------------------------------------------------------------------------------------------------------------*/
/* STRING BUILDER                                                                                                     */
/*--------------------------------------------------------------------------------------------------------------------*/

typedef struct
{
    struct indi_string_builder_node_s *head;
    struct indi_string_builder_node_s *tail;

} indi_string_builder_t;

/*--------------------------------------------------------------------------------------------------------------------*/

indi_string_builder_t *indi_string_builder_new();

void indi_string_builder_free(
    indi_string_builder_t *sb
);

void indi_string_builder_clear(
    indi_string_builder_t *sb
);

void indi_string_builder_append_n(
    indi_string_builder_t *sb,
    STR_t args[],
    size_t n
);

size_t indi_string_builder_length(
    indi_string_builder_t *sb
);

str_t indi_string_builder_to_string(
    indi_string_builder_t *sb
);

str_t indi_string_builder_to_cstring(
    indi_string_builder_t *sb
);

/*--------------------------------------------------------------------------------------------------------------------*/

#define indi_string_builder_append(sb, ...) ({                                                                         \
                                                                                                                       \
    STR_t args[] = {__VA_ARGS__};                                                                                      \
                                                                                                                       \
    indi_string_builder_append_n(sb, args, sizeof(args) / sizeof(STR_t));                                              \
})

/*--------------------------------------------------------------------------------------------------------------------*/

#define indi_string_builder_from(...) ({                                                                               \
                                                                                                                       \
    STR_t args[] = {__VA_ARGS__};                                                                                      \
                                                                                                                       \
    indi_string_builder_t *_sb = indi_string_builder_new();                                                             \
                                                                                                                       \
    indi_string_builder_append_n(_sb, args, sizeof(args) / sizeof(STR_t));                                              \
                                                                                                                       \
    _sb;                                                                                                                \
})

/*--------------------------------------------------------------------------------------------------------------------*/
/* HELPERS                                                                                                            */
/*--------------------------------------------------------------------------------------------------------------------*/

void internal_get_timestamp(str_t timestamp_buff, size_t timestamp_size);

void internal_set_opts(indi_dict_t *dict, indi_opts_t *opts);

bool internal_copy_entry(indi_dict_t *dst, const indi_dict_t *src, STR_t key);

indi_dict_t *internal_xxx_set_vector_new(const indi_dict_t *def_vector, STR_t set_tagname, STR_t one_tagname);

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

} indi_stream_t;

/*--------------------------------------------------------------------------------------------------------------------*/

#define INDI_STREAM() \
            ((indi_stream_t) {.idx = 0x00, .s_ptr = NULL, .e_ptr = NULL, .pos = 0, .len = 0})

/*--------------------------------------------------------------------------------------------------------------------*/

bool indi_stream_detect_opening_tag(
        indi_stream_t *stream,
        size_t size,
        BUFF_t buff
);

bool indi_stream_detect_closing_tag(
        indi_stream_t *stream,
        size_t size,
        BUFF_t buff
);

/*--------------------------------------------------------------------------------------------------------------------*/

#ifdef __cplusplus
}
#endif

/*--------------------------------------------------------------------------------------------------------------------*/

#endif /* INDI_BASE_INDI_BASE_INTERNAL_H */

/*--------------------------------------------------------------------------------------------------------------------*/

#pragma clang diagnostic pop

/*--------------------------------------------------------------------------------------------------------------------*/
