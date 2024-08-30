/*--------------------------------------------------------------------------------------------------------------------*/

#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCUnusedMacroInspection"
#pragma ide diagnostic ignored "bugprone-reserved-identifier"
#pragma ide diagnostic ignored "OCUnusedGlobalDeclarationInspection"

/*--------------------------------------------------------------------------------------------------------------------*/

#ifndef NYX_BASE_NYX_BASE_H
#define NYX_BASE_NYX_BASE_H

/*--------------------------------------------------------------------------------------------------------------------*/

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

/*--------------------------------------------------------------------------------------------------------------------*/

#ifdef __cplusplus
extern "C" {
#endif

/*--------------------------------------------------------------------------------------------------------------------*/

double nan(const char *tag);

/*--------------------------------------------------------------------------------------------------------------------*/

#define __INLINE__ \
            static inline

#define __NULLABLE__ \
            /* do nothing */

#define __ZEROABLE__ \
            /* do nothing */

/*--------------------------------------------------------------------------------------------------------------------*/

typedef /* */ void *buff_t;
typedef const void *BUFF_t;

typedef /* */ char *str_t;
typedef const char *STR_t;

/*--------------------------------------------------------------------------------------------------------------------*/
/* MEMORY                                                                                                             */
/*--------------------------------------------------------------------------------------------------------------------*/

void nyx_memory_initialize();

void nyx_memory_finalize();

/*--------------------------------------------------------------------------------------------------------------------*/

size_t nyx_memory_free(
    __NULLABLE__ buff_t buff
);

buff_t nyx_memory_alloc(
    __ZEROABLE__ size_t size
);

buff_t nyx_memory_realloc(
    __NULLABLE__ buff_t buff,
    __ZEROABLE__ size_t size
);

/*--------------------------------------------------------------------------------------------------------------------*/
/* OBJECT                                                                                                             */
/*--------------------------------------------------------------------------------------------------------------------*/

#define NYX_OBJECT_MAGIC 0x6565656565656565

/*--------------------------------------------------------------------------------------------------------------------*/

typedef enum nyx_type_e
{
    NYX_TYPE_NULL = 100,
    NYX_TYPE_BOOLEAN = 101,
    NYX_TYPE_NUMBER = 102,
    NYX_TYPE_STRING = 103,
    NYX_TYPE_DICT = 104,
    NYX_TYPE_LIST = 105,

} nyx_type_t;

/*--------------------------------------------------------------------------------------------------------------------*/

#define NYX_FLAGS_XXXX_DISABLED     ((uint32_t) (1 << 0))
#define NYX_FLAGS_BLOB_DISABLED     ((uint32_t) (1 << 1))
#define NYX_FLAGS_BOTH_DISABLED     (NYX_FLAGS_XXXX_DISABLED | NYX_FLAGS_BLOB_DISABLED)

/*--------------------------------------------------------------------------------------------------------------------*/

typedef struct nyx_object_s
{
    uint64_t magic;
    uint32_t flags;

    enum nyx_type_e type;

    __NULLABLE__ struct nyx_node_s *node;

    __NULLABLE__ struct nyx_object_s *parent;

    __NULLABLE__ void (* in_callback)(struct nyx_object_s *object, bool modified);
    __NULLABLE__ void (* out_callback)(struct nyx_object_s *object, bool modified);

} nyx_object_t;

/*--------------------------------------------------------------------------------------------------------------------*/

#define NYX_OBJECT(_type) \
            ((struct nyx_object_s) {.magic = NYX_OBJECT_MAGIC, .flags = 0x00, .type = _type, .node = NULL, .parent = NULL, .in_callback = NULL, .out_callback = NULL})

/*--------------------------------------------------------------------------------------------------------------------*/

nyx_object_t *nyx_object_parse(
    __NULLABLE__ STR_t text
);

void nyx_object_free(
    __NULLABLE__ /*-*/ nyx_object_t *object
);

bool nyx_object_compare(
    __NULLABLE__ const nyx_object_t *object1,
    __NULLABLE__ const nyx_object_t *object2
);

str_t nyx_object_to_string(
    __NULLABLE__ const nyx_object_t *object
);

str_t nyx_object_to_cstring(
    __NULLABLE__ const nyx_object_t *object
);

/*--------------------------------------------------------------------------------------------------------------------*/
/* NULL                                                                                                               */
/*--------------------------------------------------------------------------------------------------------------------*/

typedef struct
{
    nyx_object_t base;

} nyx_null_t;

/*--------------------------------------------------------------------------------------------------------------------*/

nyx_null_t *nyx_null_new();

void nyx_null_free(
    /*-*/ nyx_null_t *object
);

str_t nyx_null_to_string(
    const nyx_null_t *object
);

/*--------------------------------------------------------------------------------------------------------------------*/
/* NUMBER                                                                                                             */
/*--------------------------------------------------------------------------------------------------------------------*/

typedef struct
{
    nyx_object_t base;

    double value;

} nyx_number_t;

/*--------------------------------------------------------------------------------------------------------------------*/

nyx_number_t *nyx_number_new();

void nyx_number_free(
    /*-*/ nyx_number_t *object
);

double nyx_number_get(
    const nyx_number_t *object
);

void nyx_number_set(
    /*-*/ nyx_number_t *object,
    double value
);

str_t nyx_number_to_string(
    const nyx_number_t *object
);

/*--------------------------------------------------------------------------------------------------------------------*/

__INLINE__ nyx_number_t *nyx_number_from(double value)
{
    nyx_number_t *result = nyx_number_new();

    nyx_number_set(result, value);

    return result;
}

/*--------------------------------------------------------------------------------------------------------------------*/
/* BOOLEAN                                                                                                            */
/*--------------------------------------------------------------------------------------------------------------------*/

typedef struct
{
    nyx_object_t base;

    bool value;

} nyx_boolean_t;

/*--------------------------------------------------------------------------------------------------------------------*/

nyx_boolean_t *nyx_boolean_new();

void nyx_boolean_free(
    /*-*/ nyx_boolean_t *object
);

bool nyx_boolean_get(
    const nyx_boolean_t *object
);

void nyx_boolean_set(
    /*-*/ nyx_boolean_t *object,
    bool value
);

str_t nyx_boolean_to_string(
    const nyx_boolean_t *object
);

/*--------------------------------------------------------------------------------------------------------------------*/

__INLINE__ nyx_boolean_t *nyx_boolean_from(bool value)
{
    nyx_boolean_t *result = nyx_boolean_new();

    nyx_boolean_set(result, value);

    return result;
}

/*--------------------------------------------------------------------------------------------------------------------*/
/* STRING                                                                                                             */
/*--------------------------------------------------------------------------------------------------------------------*/

typedef struct
{
    nyx_object_t base;

    str_t value;

    bool dyn;

} nyx_string_t;

/*--------------------------------------------------------------------------------------------------------------------*/

nyx_string_t *nyx_string_new();

void nyx_string_free(
    /*-*/ nyx_string_t *object
);

STR_t nyx_string_get(
    const nyx_string_t *object
);

void nyx_string_dynamic_set(
    /*-*/ nyx_string_t *object,
    STR_t value
);

void nyx_string_static_set(
    /*-*/ nyx_string_t *object,
    STR_t value
);

str_t nyx_string_to_string(
    const nyx_string_t *object
);

str_t nyx_string_to_cstring(
    const nyx_string_t *object
);

/*--------------------------------------------------------------------------------------------------------------------*/

__INLINE__ nyx_string_t *nyx_string_dynamic_from(STR_t value)
{
    nyx_string_t *result = nyx_string_new();

    nyx_string_dynamic_set(result, value);

    return result;
}

/*--------------------------------------------------------------------------------------------------------------------*/

__INLINE__ nyx_string_t *nyx_string_static_from(STR_t value)
{
    nyx_string_t *result = nyx_string_new();

    nyx_string_static_set(result, value);

    return result;
}

/*--------------------------------------------------------------------------------------------------------------------*/

#define nyx_string_set nyx_string_dynamic_set

#define nyx_string_from nyx_string_dynamic_from

/*--------------------------------------------------------------------------------------------------------------------*/
/* DICT                                                                                                               */
/*--------------------------------------------------------------------------------------------------------------------*/

typedef struct
{
    nyx_object_t base;

    struct nyx_dict_node_s *head;
    struct nyx_dict_node_s *tail;

} nyx_dict_t;

/*--------------------------------------------------------------------------------------------------------------------*/

typedef struct
{
    int idx;

    nyx_type_t type;

    struct nyx_dict_node_s *head;

} nyx_dict_iter_t;

/*--------------------------------------------------------------------------------------------------------------------*/

#define NYX_DICT_ITER(object) \
                ((nyx_dict_iter_t) {.idx = 0, .type = ((nyx_dict_t *) (object))->base.type, .head = ((nyx_dict_t *) (object))->head})

/*--------------------------------------------------------------------------------------------------------------------*/

nyx_dict_t *nyx_dict_new();

void nyx_dict_free(
    /*-*/ nyx_dict_t *object
);

void nyx_dict_clear(
    /*-*/ nyx_dict_t *object
);

void nyx_dict_del(
    /*-*/ nyx_dict_t *object,
    STR_t key
);

bool nyx_dict_iterate(
    nyx_dict_iter_t *iter,
    STR_t *key,
    nyx_object_t **object
);

nyx_object_t *nyx_dict_get(
    const nyx_dict_t *object,
    STR_t key
);

void nyx_dict_set(
    /*-*/ nyx_dict_t *object,
    STR_t key,
    buff_t value
);

size_t nyx_dict_size(
    const nyx_dict_t *object
);

str_t nyx_dict_to_string(
    const nyx_dict_t *object
);

/*--------------------------------------------------------------------------------------------------------------------*/

__INLINE__ double nyx_dict_get_number(const nyx_dict_t *object, STR_t key)
{
    nyx_object_t *number = nyx_dict_get(object, key);

    return (number != NULL && number->type == NYX_TYPE_NUMBER) ? nyx_number_get((nyx_number_t *) number)
                                                                : nan("1")
    ;
}

/*--------------------------------------------------------------------------------------------------------------------*/

__INLINE__ STR_t nyx_dict_get_string(const nyx_dict_t *object, STR_t key)
{
    nyx_object_t *string = nyx_dict_get(object, key);

    return (string != NULL && string->type == NYX_TYPE_STRING) ? nyx_string_get((nyx_string_t *) string)
                                                                : NULL
    ;
}

/*--------------------------------------------------------------------------------------------------------------------*/
/* LIST                                                                                                               */
/*--------------------------------------------------------------------------------------------------------------------*/

typedef struct
{
    nyx_object_t base;

    struct nyx_list_node_s *head;
    struct nyx_list_node_s *tail;

} nyx_list_t;

/*--------------------------------------------------------------------------------------------------------------------*/

typedef struct
{
    int idx;

    nyx_type_t type;

    struct nyx_list_node_s *head;

} nyx_list_iter_t;

/*--------------------------------------------------------------------------------------------------------------------*/

#define NYX_LIST_ITER(object) \
                ((nyx_list_iter_t) {.idx = 0, .type = ((nyx_list_t *) (object))->base.type, .head = ((nyx_list_t *) (object))->head})

/*--------------------------------------------------------------------------------------------------------------------*/

nyx_list_t *nyx_list_new();

void nyx_list_free(
    /*-*/ nyx_list_t *object
);

void nyx_list_clear(
    /*-*/ nyx_list_t *object
);

void nyx_list_del(
    /*-*/ nyx_list_t *object,
    int idx
);

bool nyx_list_iterate(
    nyx_list_iter_t *iter,
    int *idx,
    nyx_object_t **object
);

nyx_object_t *nyx_list_get(
    const nyx_list_t *object,
    int idx
);

nyx_list_t *nyx_list_set(
    /*-*/ nyx_list_t *object,
    size_t idx,
    buff_t value
);

size_t nyx_list_size(
    const nyx_list_t *object
);

str_t nyx_list_to_string(
    const nyx_list_t *object
);

/*--------------------------------------------------------------------------------------------------------------------*/

__INLINE__ double nyx_list_get_number(const nyx_list_t *object, int idx)
{
    nyx_object_t *number = nyx_list_get(object, idx);

    return (number != NULL && number->type == NYX_TYPE_NUMBER) ? nyx_number_get((nyx_number_t *) number)
                                                                : nan("1")
    ;
}

/*--------------------------------------------------------------------------------------------------------------------*/

__INLINE__ STR_t nyx_list_get_string(const nyx_list_t *object, int idx)
{
    nyx_object_t *string = nyx_list_get(object, idx);

    return (string != NULL && string->type == NYX_TYPE_STRING) ? nyx_string_get((nyx_string_t *) string)
                                                                : NULL
    ;
}

/*--------------------------------------------------------------------------------------------------------------------*/

#define nyx_list_push(object, value) \
            nyx_list_set(object, -1, value)

/*--------------------------------------------------------------------------------------------------------------------*/
/* XMLDOC                                                                                                             */
/*--------------------------------------------------------------------------------------------------------------------*/

typedef struct _xmlDoc nyx_xmldoc_t;

/*--------------------------------------------------------------------------------------------------------------------*/

nyx_xmldoc_t *nyx_xmldoc_parse_buff(
    __NULLABLE__ BUFF_t buff,
    __ZEROABLE__ size_t size
);

nyx_xmldoc_t *nyx_xmldoc_parse(
    __NULLABLE__ STR_t text
);

void nyx_xmldoc_free(
    __NULLABLE__ /*-*/ nyx_xmldoc_t *xmldoc
);

str_t nyx_xmldoc_to_string(
    __NULLABLE__ const nyx_xmldoc_t *xmldoc
);

/*--------------------------------------------------------------------------------------------------------------------*/
/* VALIDATION                                                                                                         */
/*--------------------------------------------------------------------------------------------------------------------*/

bool nyx_validation_initialize();

bool nyx_validation_finalize();

bool nyx_validation_check(
    __NULLABLE__ const nyx_xmldoc_t *doc
);

/*--------------------------------------------------------------------------------------------------------------------*/
/* TRANSFORM                                                                                                          */
/*--------------------------------------------------------------------------------------------------------------------*/

nyx_object_t *nyx_xmldoc_to_object(
    __NULLABLE__ const nyx_xmldoc_t *xmldoc,
    bool validate
);

nyx_xmldoc_t *nyx_object_to_xmldoc(
    __NULLABLE__ const nyx_object_t *object,
    bool validate
);

/*--------------------------------------------------------------------------------------------------------------------*/
/* NYX                                                                                                               */
/*--------------------------------------------------------------------------------------------------------------------*/

#define NYX_VERSION "1.7"

/*--------------------------------------------------------------------------------------------------------------------*/

typedef enum
{
    NYX_STATE_IDLE = 200,
    NYX_STATE_OK = 201,
    NYX_STATE_BUSY = 202,
    NYX_STATE_ALERT = 203,

} nyx_state_t;

STR_t nyx_state_to_str(
    nyx_state_t state
);

nyx_state_t nyx_str_to_state(
    STR_t state
);

/*--------------------------------------------------------------------------------------------------------------------*/

typedef enum
{
    NYX_PERM_RO = 300,
    NYX_PERM_WO = 301,
    NYX_PERM_RW = 302,

} nyx_perm_t;

STR_t nyx_perm_to_str(
    nyx_perm_t perm
);

nyx_perm_t nyx_str_to_perm(
    STR_t perm
);

/*--------------------------------------------------------------------------------------------------------------------*/

typedef enum
{
    NYX_RULE_ONE_OF_MANY = 400,
    NYX_RULE_AT_MOST_ONE = 401,
    NYX_RULE_ANY_OF_MANY = 402,

} nyx_rule_t;

STR_t nyx_rule_to_str(
    nyx_rule_t rule
);

nyx_rule_t nyx_str_to_rule(
    STR_t rule
);

/*--------------------------------------------------------------------------------------------------------------------*/

typedef enum
{
    NYX_ONOFF_ON = 500,
    NYX_ONOFF_OFF = 501,

} nyx_onoff_t;

STR_t nyx_onoff_to_str(
    nyx_onoff_t onoff
);

nyx_onoff_t nyx_str_to_onoff(
    STR_t onoff
);

/*--------------------------------------------------------------------------------------------------------------------*/

typedef enum
{
    NYX_BLOB_NEVER = 600,
    NYX_BLOB_ALSO = 601,
    NYX_BLOB_ONLY = 602,

} nyx_blob_t;

STR_t nyx_blob_to_str(
    nyx_blob_t blob
);

nyx_blob_t nyx_str_to_blob(
    STR_t blob
);

/*--------------------------------------------------------------------------------------------------------------------*/

typedef struct
{
    __NULLABLE__ STR_t label;
    __NULLABLE__ STR_t group;
    __NULLABLE__ double timeout;
    __NULLABLE__ STR_t message;

} nyx_opts_t;

/*--------------------------------------------------------------------------------------------------------------------*/

nyx_dict_t *nyx_number_def_new(
    STR_t name,
    __NULLABLE__ STR_t label,
    STR_t format,
    float min,
    float max,
    float step,
    float value
);

#define nyx_number_def_set(def, value) \
            nyx_dict_set(def, "$", nyx_number_from(value))

#define nyx_number_def_get(def) \
            ((nyx_number_t *) nyx_dict_get(def, "$"))->data

nyx_dict_t *nyx_number_def_vector_new(
    STR_t device,
    STR_t name,
    nyx_state_t state,
    nyx_perm_t perm,
    nyx_dict_t *defs[],
    nyx_opts_t *opts
);

nyx_dict_t *nyx_number_set_vector_new(
    const nyx_dict_t *def_vector
);

/*--------------------------------------------------------------------------------------------------------------------*/

nyx_dict_t *nyx_text_def_new(
    STR_t name,
    __NULLABLE__ STR_t label,
    STR_t value
);

#define nyx_text_def_set(def, value) \
            nyx_dict_set(def, "$", nyx_string_from(value))

#define nyx_text_def_get(def) \
            ((nyx_string_t *) nyx_dict_get(def, "$"))->data

nyx_dict_t *nyx_text_def_vector_new(
    STR_t device,
    STR_t name,
    nyx_state_t state,
    nyx_perm_t perm,
    nyx_dict_t *defs[],
    nyx_opts_t *opts
);

nyx_dict_t *nyx_text_set_vector_new(
    const nyx_dict_t *def_vector
);

/*--------------------------------------------------------------------------------------------------------------------*/

nyx_dict_t *nyx_light_def_new(
    STR_t name,
    __NULLABLE__ STR_t label,
    nyx_state_t value
);

#define nyx_light_def_set(def, value) \
            nyx_dict_set(def, "$", nyx_string_from(nyx_state_to_str(value)))

#define nyx_light_def_get(def) \
            nyx_str_to_state(((nyx_string_t *) nyx_dict_get(def, "$"))->data)

nyx_dict_t *nyx_light_def_vector_new(
    STR_t device,
    STR_t name,
    nyx_state_t state,
    nyx_dict_t *defs[],
    nyx_opts_t *opts
);

nyx_dict_t *nyx_light_set_vector_new(
    const nyx_dict_t *def_vector
);

/*--------------------------------------------------------------------------------------------------------------------*/

nyx_dict_t *nyx_switch_def_new(
    STR_t name,
    __NULLABLE__ STR_t label,
    nyx_onoff_t value
);

#define nyx_switch_def_set(def, value) \
            nyx_dict_set(def, "$", nyx_string_from(nyx_onoff_to_str(value)))

#define nyx_switch_def_get(def) \
            nyx_str_to_onoff(((nyx_string_t *) nyx_dict_get(def, "$"))->value)

nyx_dict_t *nyx_switch_def_vector_new(
    STR_t device,
    STR_t name,
    nyx_state_t state,
    nyx_perm_t perm,
    nyx_rule_t rule,
    nyx_dict_t *defs[],
    nyx_opts_t *opts
);

nyx_dict_t *nyx_switch_set_vector_new(
    const nyx_dict_t *def_vector
);

/*--------------------------------------------------------------------------------------------------------------------*/

nyx_dict_t *nyx_blob_def_new(
    STR_t name,
    __NULLABLE__ STR_t label,
    STR_t value
);

#define nyx_blob_def_set(def, value) \
            nyx_dict_set(def, "$", nyx_string_from(value))

#define nyx_blob_def_get(def) \
            ((nyx_string_t *) nyx_dict_get(def, "$"))->data

nyx_dict_t *nyx_blob_def_vector_new(
    STR_t device,
    STR_t name,
    nyx_state_t state,
    nyx_perm_t perm,
    nyx_dict_t *defs[],
    nyx_opts_t *opts
);

nyx_dict_t *nyx_blob_set_vector_new(
    const nyx_dict_t *def_vector
);

/*--------------------------------------------------------------------------------------------------------------------*/

nyx_dict_t *nyx_message_new(
    STR_t device,
    STR_t message
);

nyx_dict_t *nyx_del_property_new(
    STR_t device,
    __NULLABLE__ STR_t name,
    __NULLABLE__ STR_t message
);

/*--------------------------------------------------------------------------------------------------------------------*/
/* SERVER                                                                                                             */
/*--------------------------------------------------------------------------------------------------------------------*/

#define INI_PING_MS 5000

/*--------------------------------------------------------------------------------------------------------------------*/

typedef struct nyx_node_s nyx_node_t;

/*--------------------------------------------------------------------------------------------------------------------*/

nyx_node_t *nyx_node_initialize(
    __NULLABLE__ STR_t tcp_url,
    __NULLABLE__ STR_t mqtt_url,
    __NULLABLE__ STR_t username,
    __NULLABLE__ STR_t password,
    /**/
    STR_t node_id,
    nyx_dict_t *def_vectors[],
    /**/
    int retry_ms,
    bool enable_xml,
    bool validate_xml
);

void nyx_node_pool(
    nyx_node_t *node,
    int timeout_ms
);

void nyx_node_free(
    nyx_node_t *node,
    bool free_vectors
);

/*--------------------------------------------------------------------------------------------------------------------*/

void nyx_node_send_message(
    nyx_node_t *node,
    STR_t device,
    STR_t message
);

/*--------------------------------------------------------------------------------------------------------------------*/

void nyx_node_send_del_property(
    nyx_node_t *node,
    STR_t device,
    __NULLABLE__ STR_t name,
    __NULLABLE__ STR_t message
);

/*--------------------------------------------------------------------------------------------------------------------*/

#ifdef __cplusplus
}
#endif

/*--------------------------------------------------------------------------------------------------------------------*/

#endif /* NYX_BASE_NYX_BASE_H */

/*--------------------------------------------------------------------------------------------------------------------*/

#pragma clang diagnostic pop

/*--------------------------------------------------------------------------------------------------------------------*/
