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
/* OBJECT                                                                                                             */
/*--------------------------------------------------------------------------------------------------------------------*/

#define NYX_FLAGS_BOTH_DISABLED     (NYX_FLAGS_XXXX_DISABLED | NYX_FLAGS_BLOB_DISABLED)

/*--------------------------------------------------------------------------------------------------------------------*/

void nyx_object_notify(
    __NULLABLE__ nyx_object_t *object,
    bool modified
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
    /*-*/ nyx_string_builder_t *sb
);

void nyx_string_builder_clear(
    /*-*/ nyx_string_builder_t *sb
);

void nyx_string_builder_append_n(
    /*-*/ nyx_string_builder_t *sb,
    STR_t args[],
    size_t n
);

size_t nyx_string_builder_length(
    const nyx_string_builder_t *sb
);

str_t nyx_string_builder_to_string(
    const nyx_string_builder_t *sb
);

str_t nyx_string_builder_to_cstring(
    const nyx_string_builder_t *sb
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

void internal_get_timestamp(
    str_t timestamp_buff,
    size_t timestamp_size
);

void internal_set_opts(
    nyx_dict_t *dict,
    nyx_opts_t *opts
);

bool internal_copy(
    /*-*/ nyx_dict_t *dst,
    const nyx_dict_t *src,
    STR_t key,
    bool notify
);

nyx_dict_t *internal_xxxx_set_vector_new(
    const nyx_dict_t *def_vector,
    STR_t set_tag,
    STR_t one_tag
);

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
