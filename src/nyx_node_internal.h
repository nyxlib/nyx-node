/*--------------------------------------------------------------------------------------------------------------------*/

#ifndef NYX_NODE_INTERNAL_H
#define NYX_NODE_INTERNAL_H

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
/* STRING                                                                                                             */
/*--------------------------------------------------------------------------------------------------------------------*/

#ifndef MG_VERSION
typedef struct mg_str {
    str_t buf;
    size_t len;
} nyx_str_t;
#else
typedef struct mg_str nyx_str_t;
#endif

/*--------------------------------------------------------------------------------------------------------------------*/

nyx_str_t nyx_str_s(STR_t s);

/*--------------------------------------------------------------------------------------------------------------------*/
/* HASH-32                                                                                                            */
/*--------------------------------------------------------------------------------------------------------------------*/

uint32_t nyx_hash32(
    BUFF_t buff,
    size_t size,
    uint32_t seed
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

#define NYX_OBJECT(Type) \
            ((struct nyx_object_s) {            \
                .magic = NYX_OBJECT_MAGIC,      \
                .flags = 0x00000000000000,      \
                .type = Type,                   \
                .node = NULL,                   \
                .parent = NULL,                 \
                .in_callback = NULL,            \
                .out_callback = NULL            \
            })

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
    size_t n,
    bool xml
);

size_t nyx_string_builder_length(
    const nyx_string_builder_t *sb
);

size_t nyx_string_builder_clength(
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
    nyx_string_builder_append_n(sb, args, sizeof(args) / sizeof(STR_t), false);                                        \
})

/*--------------------------------------------------------------------------------------------------------------------*/

#define nyx_string_builder_append_xml(sb, ...) ({                                                                      \
                                                                                                                       \
    STR_t args[] = {__VA_ARGS__};                                                                                      \
                                                                                                                       \
    nyx_string_builder_append_n(sb, args, sizeof(args) / sizeof(STR_t), true);                                         \
})

/*--------------------------------------------------------------------------------------------------------------------*/

#define nyx_string_builder_from(...) ({                                                                                \
                                                                                                                       \
    STR_t args[] = {__VA_ARGS__};                                                                                      \
                                                                                                                       \
    nyx_string_builder_t *_sb = nyx_string_builder_new();                                                              \
                                                                                                                       \
    nyx_string_builder_append_n(_sb, args, sizeof(args) / sizeof(STR_t), false);                                       \
                                                                                                                       \
    _sb;                                                                                                               \
})

/*--------------------------------------------------------------------------------------------------------------------*/

#define nyx_string_builder_from_xml(...) ({                                                                            \
                                                                                                                       \
    STR_t args[] = {__VA_ARGS__};                                                                                      \
                                                                                                                       \
    nyx_string_builder_t *_sb = nyx_string_builder_new();                                                              \
                                                                                                                       \
    nyx_string_builder_append_n(_sb, args, sizeof(args) / sizeof(STR_t), true);                                        \
                                                                                                                       \
    _sb;                                                                                                               \
})

/*--------------------------------------------------------------------------------------------------------------------*/
/* DOM                                                                                                                */
/*--------------------------------------------------------------------------------------------------------------------*/

nyx_xmldoc_t *nyx_xmldoc_new(
    nyx_xml_type_t type
);

/*--------------------------------------------------------------------------------------------------------------------*/

__NULLABLE__ str_t nyx_xmldoc_get_name(
    const nyx_xmldoc_t *xmldoc
);

void nyx_xmldoc_set_name(
    nyx_xmldoc_t *xmldoc,
    __NULLABLE__ STR_t name
);

__NULLABLE__ str_t nyx_xmldoc_get_content(
    const nyx_xmldoc_t *xmldoc
);

void nyx_xmldoc_set_content(
    nyx_xmldoc_t *xmldoc,
    __NULLABLE__ STR_t data
);

/*--------------------------------------------------------------------------------------------------------------------*/

void nyx_xmldoc_add_child(
    nyx_xmldoc_t *xmldoc,
    __NULLABLE__ nyx_xmldoc_t *child
);

void nyx_xmldoc_add_attribute(
    nyx_xmldoc_t *xmldoc,
    __NULLABLE__ STR_t name,
    __NULLABLE__ STR_t data
);

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
/* NODE                                                                                                               */
/*--------------------------------------------------------------------------------------------------------------------*/

void nyx_mqtt_sub(
    nyx_node_t *node,
    nyx_str_t topic,
    int qos
);

void nyx_mqtt_pub(
    nyx_node_t *node,
    nyx_str_t topic,
    nyx_str_t message,
    int qos,
    bool retain
);

void nyx_tcp_pub(
    nyx_node_t *node,
    STR_t message
);

/*--------------------------------------------------------------------------------------------------------------------*/

#define NYX_PING_MS 5000

#define NYX_EVENT_OPEN  0
#define NYX_EVENT_MSG   1

typedef struct nyx_stack_s nyx_stack_t;

struct nyx_node_s
{
    nyx_str_t node_id;

    STR_t tcp_url;
    STR_t mqtt_url;

    nyx_stack_t *stack;

    nyx_dict_t **def_vectors;

    /**/

    size_t (* tcp_handler)(struct nyx_node_s *node, int event_type, size_t size, BUFF_t buff);

    void (* mqtt_handler)(struct nyx_node_s *node, int event_type, nyx_str_t event_topic, nyx_str_t event_message);

    /**/

    bool enable_xml;
    bool validate_xml;

    int last_ping_ms;
};

void nyx_node_stack_initialize(
    nyx_node_t *node,
    __NULLABLE__ STR_t mqtt_username,
    __NULLABLE__ STR_t mqtt_password,
    int retry_ms
);

void nyx_node_stack_finalize(
    nyx_node_t *node
);

void nyx_stack_poll(
    nyx_node_t *node,
    int timeout_ms
);

/*--------------------------------------------------------------------------------------------------------------------*/

#ifdef __cplusplus
}
#endif

/*--------------------------------------------------------------------------------------------------------------------*/

#endif /* NYX_NODE_INTERNAL_H */

/*--------------------------------------------------------------------------------------------------------------------*/
