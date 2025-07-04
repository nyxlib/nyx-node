/* NyxNode
 * Author: Jérôme ODIER <jerome.odier@lpsc.in2p3.fr>
 * SPDX-License-Identifier: GPL-2.0-only (Mongoose backend) or GPL-3.0+
 */

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

#ifndef ARDUINO
#  pragma clang diagnostic push
#  pragma ide diagnostic ignored "OCUnusedMacroInspection"
#  pragma ide diagnostic ignored "UnreachableCallsOfFunction"
#endif

/*--------------------------------------------------------------------------------------------------------------------*/
/* MEMORY                                                                                                             */
/*--------------------------------------------------------------------------------------------------------------------*/

str_t nyx_boolean_dup(
    bool b
);

__NULLABLE__ str_t nyx_double_dup(
    double d
);

__NULLABLE__ str_t nyx_string_dup(
    __NULLABLE__ STR_t s
);

__NULLABLE__ str_t nyx_string_ndup(
    __NULLABLE__ STR_t s,
    __ZEROABLE__ size_t n
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

#define NYX_STR_S(_buf, _len) \
            ((nyx_str_t) {.buf = (str_t) (_buf), .len = (size_t) (_len)})

/*--------------------------------------------------------------------------------------------------------------------*/

nyx_str_t nyx_str_s(
    __NULLABLE__ STR_t s
);

/*--------------------------------------------------------------------------------------------------------------------*/
/* LOGGER                                                                                                             */
/*--------------------------------------------------------------------------------------------------------------------*/

extern nyx_log_level_t nyx_log_level;

/*--------------------------------------------------------------------------------------------------------------------*/

STR_t nyx_log_level_to_str(
    nyx_log_level_t level
);

/*--------------------------------------------------------------------------------------------------------------------*/
/* UTF-8                                                                                                              */
/*--------------------------------------------------------------------------------------------------------------------*/

int nyx_unicode_to_utf8(
    str_t result,
    uint32_t unicode_char
);

/*--------------------------------------------------------------------------------------------------------------------*/
/* OBJECT                                                                                                             */
/*--------------------------------------------------------------------------------------------------------------------*/

#define NYX_OBJECT(Type) \
            ((nyx_object_t) {                   \
                .magic = NYX_OBJECT_MAGIC,      \
                .flags = 0x00000000000000,      \
                .type = (Type),                 \
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
    /*-*/ nyx_xmldoc_t *xmldoc,
    __NULLABLE__ STR_t name
);

__NULLABLE__ str_t nyx_xmldoc_get_content(
    const nyx_xmldoc_t *xmldoc
);

void nyx_xmldoc_set_content(
    /*-*/ nyx_xmldoc_t *xmldoc,
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

int internal_get_timestamp(
    str_t buff,
    size_t size
);

void internal_set_opts(
    /*--------*/ /*-*/ nyx_dict_t *dict,
    __NULLABLE__ const nyx_opts_t *opts
);

bool internal_copy(
    /*-*/ nyx_dict_t *dst,
    const nyx_dict_t *src,
    STR_t key,
    bool notify
);

nyx_dict_t *internal_def_to_set(
    const nyx_dict_t *def_vector,
    STR_t set_tag,
    STR_t one_tag
);

/*--------------------------------------------------------------------------------------------------------------------*/
/* XML STREAM                                                                                                         */
/*--------------------------------------------------------------------------------------------------------------------*/

typedef struct
{
    struct tag_s *tag;

    STR_t s_ptr;
    STR_t e_ptr;

    size_t pos;
    size_t len;

} nyx_xml_stream_t;

/*--------------------------------------------------------------------------------------------------------------------*/

#define NYX_XML_STREAM() \
            ((nyx_xml_stream_t) {.tag = NULL, .s_ptr = NULL, .e_ptr = NULL, .pos = 0, .len = 0})

/*--------------------------------------------------------------------------------------------------------------------*/

bool nyx_xml_stream_detect_opening_tag(
    nyx_xml_stream_t *xml_stream,
    size_t size,
    BUFF_t buff
);

bool nyx_xml_stream_detect_closing_tag(
    nyx_xml_stream_t *xml_stream,
    size_t size,
    BUFF_t buff
);

/*--------------------------------------------------------------------------------------------------------------------*/
/* NODE                                                                                                               */
/*--------------------------------------------------------------------------------------------------------------------*/

#define NYX_PING_MS 5000UL

#define NYX_ALL "@ALL"

/*--------------------------------------------------------------------------------------------------------------------*/

typedef struct nyx_stack_s nyx_stack_t;

/*--------------------------------------------------------------------------------------------------------------------*/

struct nyx_node_s
{
    nyx_str_t node_id;

    nyx_str_t master_client_topic;
    nyx_str_t master_client_message;

    /**/

    STR_t indi_url;
    STR_t mqtt_url;
    STR_t redis_url;

    bool enable_xml;
    bool validate_xml;

    /**/

    nyx_stack_t *stack;

    nyx_dict_t **def_vectors;

    __ZEROABLE__ uint32_t client_hashes[31];

    /**/

    size_t (* tcp_handler)(
        struct nyx_node_s *node,
        nyx_event_t event_type,
        const nyx_str_t payload
    );

    void (* mqtt_handler)(
        struct nyx_node_s *node,
        nyx_event_t event_type,
        const nyx_str_t topic,
        const nyx_str_t payload
    );

    /**/

    nyx_mqtt_handler_t user_mqtt_handler;
};

/*--------------------------------------------------------------------------------------------------------------------*/

void internal_indi_pub(
    /*-*/ nyx_node_t *node,
    const nyx_str_t message
);

void internal_mqtt_sub(
    /*-*/ nyx_node_t *node,
    const nyx_str_t topic
);

void internal_mqtt_pub(
    /*-*/ nyx_node_t *node,
    const nyx_str_t topic,
    const nyx_str_t message
);

void internal_redis_pub(
    /*-*/ nyx_node_t *node,
    const nyx_str_t message
);

/*--------------------------------------------------------------------------------------------------------------------*/

void nyx_node_ping(
    nyx_node_t *node
);

/*--------------------------------------------------------------------------------------------------------------------*/

void nyx_node_stack_initialize(
    nyx_node_t *node,
    __NULLABLE__ STR_t mqtt_username,
    __NULLABLE__ STR_t mqtt_password,
    __NULLABLE__ STR_t redis_username,
    __NULLABLE__ STR_t redis_password,
    int retry_ms
);

void nyx_node_stack_finalize(
    nyx_node_t *node
);

/*--------------------------------------------------------------------------------------------------------------------*/
/* REDIS                                                                                                              */
/*--------------------------------------------------------------------------------------------------------------------*/

void nyx_redis_auth(
    nyx_node_t *node,
    __NULLABLE__ STR_t username,
    __NULLABLE__ STR_t password
);

/*--------------------------------------------------------------------------------------------------------------------*/

void nyx_redis_pub(
    nyx_node_t *node,
    STR_t device,
    STR_t stream,
    size_t max_len,
    __ZEROABLE__ size_t n_fields,
    const str_t field_names[],
    const size_t field_sizes[],
    const buff_t field_buffs[]
);

/*--------------------------------------------------------------------------------------------------------------------*/

#ifndef ARDUINO
#  pragma clang diagnostic pop
#endif

/*--------------------------------------------------------------------------------------------------------------------*/

#ifdef __cplusplus
}
#endif

/*--------------------------------------------------------------------------------------------------------------------*/

#endif /* NYX_NODE_INTERNAL_H */

/*--------------------------------------------------------------------------------------------------------------------*/
