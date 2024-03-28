/*--------------------------------------------------------------------------------------------------------------------*/

#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCUnusedMacroInspection"
#pragma ide diagnostic ignored "bugprone-reserved-identifier"
#pragma ide diagnostic ignored "OCUnusedGlobalDeclarationInspection"

/*--------------------------------------------------------------------------------------------------------------------*/

#ifndef INDI_BASE_INDI_BASE_H
#define INDI_BASE_INDI_BASE_H

/*--------------------------------------------------------------------------------------------------------------------*/

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

/*--------------------------------------------------------------------------------------------------------------------*/

#define __INLINE__ \
            static inline

#define __NULLABLE__ \
            /* do nothing */

/*--------------------------------------------------------------------------------------------------------------------*/

typedef /* */ void *buff_t;
typedef const void *BUFF_t;

typedef /* */ char *str_t;
typedef const char *STR_t;

/*--------------------------------------------------------------------------------------------------------------------*/
/* MEMORY                                                                                                             */
/*--------------------------------------------------------------------------------------------------------------------*/

void indi_memory_initialize();

void indi_memory_finalize();

/*--------------------------------------------------------------------------------------------------------------------*/

size_t indi_memory_free(
    buff_t buff
);

buff_t indi_memory_alloc(
    size_t size
);

buff_t indi_memory_realloc(
    buff_t buff,
    size_t size
);

/*--------------------------------------------------------------------------------------------------------------------*/
/* OBJECT                                                                                                             */
/*--------------------------------------------------------------------------------------------------------------------*/

#define INDI_OBJECT_MAGIC 0x6565656565656565

/*--------------------------------------------------------------------------------------------------------------------*/

typedef enum indi_type_e
{
    INDI_TYPE_NULL = 100,
    INDI_TYPE_BOOLEAN = 101,
    INDI_TYPE_NUMBER = 102,
    INDI_TYPE_STRING = 103,
    INDI_TYPE_DICT = 104,
    INDI_TYPE_LIST = 105,

} indi_type_t;

/*--------------------------------------------------------------------------------------------------------------------*/

typedef struct indi_object_s
{
    uint64_t magic;

    enum indi_type_e type;

    __NULLABLE__ struct indi_object_s *parent;

    __NULLABLE__ struct indi_server_ctx_s *server_ctx;

    __NULLABLE__ void (* in_callback)(const struct indi_object_s *object);
    __NULLABLE__ void (* out_callback)(const struct indi_object_s *object);

} indi_object_t;

/*--------------------------------------------------------------------------------------------------------------------*/

#define INDI_OBJECT(_type) \
            ((struct indi_object_s) {.magic = INDI_OBJECT_MAGIC, .type = _type, .parent = NULL, .server_ctx = NULL, .in_callback = NULL, .out_callback = NULL})

/*--------------------------------------------------------------------------------------------------------------------*/

indi_object_t *indi_object_parse(
    __NULLABLE__ STR_t text
);

void indi_object_free(
    /*-*/ indi_object_t *object
);

str_t indi_object_to_string(
    const indi_object_t *object
);

str_t indi_object_to_cstring(
    const indi_object_t *object
);

/*--------------------------------------------------------------------------------------------------------------------*/
/* NULL                                                                                                               */
/*--------------------------------------------------------------------------------------------------------------------*/

typedef struct
{
    indi_object_t base;

} indi_null_t;

/*--------------------------------------------------------------------------------------------------------------------*/

indi_null_t *indi_null_new();

void indi_null_free(
    /*-*/ indi_null_t *object
);

str_t indi_null_to_string(
    const indi_null_t *object
);

/*--------------------------------------------------------------------------------------------------------------------*/
/* NUMBER                                                                                                             */
/*--------------------------------------------------------------------------------------------------------------------*/

typedef struct
{
    indi_object_t base;

    double value;

} indi_number_t;

/*--------------------------------------------------------------------------------------------------------------------*/

indi_number_t *indi_number_new();

void indi_number_free(
    /*-*/ indi_number_t *object
);

double indi_number_get(
    const indi_number_t *object
);

void indi_number_set(
    /*-*/ indi_number_t *object,
    double value
);

str_t indi_number_to_string(
    const indi_number_t *object
);

/*--------------------------------------------------------------------------------------------------------------------*/

__INLINE__ indi_number_t *indi_number_from(double value)
{
    indi_number_t *result = indi_number_new();

    indi_number_set(result, value);

    return result;
}

/*--------------------------------------------------------------------------------------------------------------------*/
/* BOOLEAN                                                                                                            */
/*--------------------------------------------------------------------------------------------------------------------*/

typedef struct
{
    indi_object_t base;

    bool value;

} indi_boolean_t;

/*--------------------------------------------------------------------------------------------------------------------*/

indi_boolean_t *indi_boolean_new();

void indi_boolean_free(
    /*-*/ indi_boolean_t *object
);

bool indi_boolean_get(
    const indi_boolean_t *object
);

void indi_boolean_set(
    /*-*/ indi_boolean_t *object,
    bool value
);

str_t indi_boolean_to_string(
    const indi_boolean_t *object
);

/*--------------------------------------------------------------------------------------------------------------------*/

__INLINE__ indi_boolean_t *indi_boolean_from(bool value)
{
    indi_boolean_t *result = indi_boolean_new();

    indi_boolean_set(result, value);

    return result;
}

/*--------------------------------------------------------------------------------------------------------------------*/
/* STRING                                                                                                             */
/*--------------------------------------------------------------------------------------------------------------------*/

typedef struct
{
    indi_object_t base;

    str_t value;

} indi_string_t;

/*--------------------------------------------------------------------------------------------------------------------*/

indi_string_t *indi_string_new();

void indi_string_free(
    /*-*/ indi_string_t *object
);

STR_t indi_string_get(
    const indi_string_t *object
);

void indi_string_set(
    /*-*/ indi_string_t *object,
    STR_t value
);

str_t indi_string_to_string(
    const indi_string_t *object
);

str_t indi_string_to_cstring(
    const indi_string_t *object
);

/*--------------------------------------------------------------------------------------------------------------------*/

__INLINE__ indi_string_t *indi_string_from(STR_t value)
{
    indi_string_t *result = indi_string_new();

    indi_string_set(result, value);

    return result;
}

/*--------------------------------------------------------------------------------------------------------------------*/
/* DICT                                                                                                               */
/*--------------------------------------------------------------------------------------------------------------------*/

typedef struct
{
    indi_object_t base;

    struct indi_dict_node_s *head;
    struct indi_dict_node_s *tail;

} indi_dict_t;

/*--------------------------------------------------------------------------------------------------------------------*/

typedef struct
{
    int idx;

    indi_type_t type;

    struct indi_dict_node_s *head;

} indi_dict_iter_t;

/*--------------------------------------------------------------------------------------------------------------------*/

#define INDI_DICT_ITER(object) \
                ((indi_dict_iter_t) {.idx = 0, .type = ((indi_dict_t *) (object))->base.type, .head = ((indi_dict_t *) (object))->head})

/*--------------------------------------------------------------------------------------------------------------------*/

indi_dict_t *indi_dict_new();

void indi_dict_free(
    /*-*/ indi_dict_t *object
);

void indi_dict_clear(
    /*-*/ indi_dict_t *object
);

void indi_dict_del(
    /*-*/ indi_dict_t *object,
    STR_t key
);

bool indi_dict_iterate(
    indi_dict_iter_t *iter,
    STR_t *key,
    indi_object_t **object
);

indi_object_t *indi_dict_get(
    const indi_dict_t *object,
    STR_t key
);

void indi_dict_set(
    /*-*/ indi_dict_t *object,
    STR_t key,
    buff_t value
);

size_t indi_dict_size(
    const indi_dict_t *object
);

str_t indi_dict_to_string(
    const indi_dict_t *object
);

/*--------------------------------------------------------------------------------------------------------------------*/
/* LIST                                                                                                               */
/*--------------------------------------------------------------------------------------------------------------------*/

typedef struct
{
    indi_object_t base;

    struct indi_list_node_s *head;
    struct indi_list_node_s *tail;

} indi_list_t;

/*--------------------------------------------------------------------------------------------------------------------*/

typedef struct
{
    int idx;

    indi_type_t type;

    struct indi_list_node_s *head;

} indi_list_iter_t;

/*--------------------------------------------------------------------------------------------------------------------*/

#define INDI_LIST_ITER(object) \
                ((indi_list_iter_t) {.idx = 0, .type = ((indi_list_t *) (object))->base.type, .head = ((indi_list_t *) (object))->head})

/*--------------------------------------------------------------------------------------------------------------------*/

indi_list_t *indi_list_new();

void indi_list_free(
    /*-*/ indi_list_t *object
);

void indi_list_clear(
    /*-*/ indi_list_t *object
);

void indi_list_del(
    /*-*/ indi_list_t *object,
    int idx
);

bool indi_list_iterate(
    indi_list_iter_t *iter,
    int *idx,
    indi_object_t **object
);

indi_object_t *indi_list_get(
    const indi_list_t *object,
    int idx
);

indi_list_t *indi_list_set(
    /*-*/ indi_list_t *object,
    size_t idx,
    buff_t value
);

size_t indi_list_size(
    const indi_list_t *object
);

str_t indi_list_to_string(
    const indi_list_t *object
);

/*--------------------------------------------------------------------------------------------------------------------*/

#define indi_list_push(object, val) \
            indi_list_set(object, -1, val)

/*--------------------------------------------------------------------------------------------------------------------*/
/* XMLDOC                                                                                                             */
/*--------------------------------------------------------------------------------------------------------------------*/

typedef struct _xmlDoc indi_xmldoc_t;

/*--------------------------------------------------------------------------------------------------------------------*/

indi_xmldoc_t *indi_xmldoc_parse(
    __NULLABLE__ STR_t text
);

void indi_xmldoc_free(
    /*-*/ indi_xmldoc_t *doc
);

str_t indi_xmldoc_to_string(
    const indi_xmldoc_t *doc
);

/*--------------------------------------------------------------------------------------------------------------------*/
/* INDI                                                                                                               */
/*--------------------------------------------------------------------------------------------------------------------*/

typedef enum
{
    INDI_STATE_IDLE = 200,
    INDI_STATE_OK = 201,
    INDI_STATE_BUSY = 202,
    INDI_STATE_ALERT = 203,

} indi_state_t;

STR_t indi_state_to_str(
    indi_state_t state
);

indi_state_t indi_str_to_state(
    STR_t state
);

/*--------------------------------------------------------------------------------------------------------------------*/

typedef enum
{
    INDI_PERM_RO = 300,
    INDI_PERM_WO = 301,
    INDI_PERM_RW = 302,

} indi_perm_t;

STR_t indi_perm_to_str(
    indi_perm_t perm
);

indi_perm_t indi_str_to_perm(
    STR_t perm
);

/*--------------------------------------------------------------------------------------------------------------------*/

typedef enum
{
    INDI_RULE_ONE_OF_MANY = 400,
    INDI_RULE_AT_MOST_ONE = 401,
    INDI_RULE_ANY_OF_MANY = 402,

} indi_rule_t;

STR_t indi_rule_to_str(
    indi_rule_t rule
);

indi_rule_t indi_str_to_rule(
    STR_t rule
);

/*--------------------------------------------------------------------------------------------------------------------*/

typedef enum
{
    INDI_ONOFF_ON = 500,
    INDI_ONOFF_OFF = 501,

} indi_onoff_t;

STR_t indi_onoff_to_str(
    indi_onoff_t onoff
);

indi_onoff_t indi_str_to_onoff(
    STR_t onoff
);

/*--------------------------------------------------------------------------------------------------------------------*/

typedef struct
{
    __NULLABLE__ STR_t label;
    __NULLABLE__ STR_t group;
    __NULLABLE__ double timeout;
    __NULLABLE__ STR_t timestamp;
    __NULLABLE__ STR_t message;

} indi_opt_t;

/*--------------------------------------------------------------------------------------------------------------------*/

indi_dict_t *indi_number_def_new(
    STR_t name,
    __NULLABLE__ STR_t label,
    STR_t format,
    float min,
    float max,
    float step,
    float value
);

#define indi_number_def_set(def, value) \
            indi_dict_set(def, "$", indi_number_from(value))

#define indi_number_def_get(def) \
            ((indi_number_t *) indi_dict_get(def, "$"))->data

indi_dict_t *indi_number_def_vector_new(
    STR_t device,
    STR_t name,
    indi_perm_t perm,
    indi_state_t state,
    indi_dict_t *defs[],
    indi_opt_t *opt
);

indi_dict_t *indi_number_set_vector_new(
    indi_dict_t *def_vector
);

/*--------------------------------------------------------------------------------------------------------------------*/

indi_dict_t *indi_text_def_new(
    STR_t name,
    __NULLABLE__ STR_t label,
    STR_t value
);

#define indi_text_def_set(def, value) \
            indi_dict_set(def, "$", indi_string_from(value))

#define indi_text_def_get(def) \
            ((indi_string_t *) indi_dict_get(def, "$"))->data

indi_dict_t *indi_text_def_vector_new(
    STR_t device,
    STR_t name,
    indi_perm_t perm,
    indi_state_t state,
    indi_dict_t *defs[],
    indi_opt_t *opt
);

indi_dict_t *indi_text_set_vector_new(
    indi_dict_t *def_vector
);

/*--------------------------------------------------------------------------------------------------------------------*/

indi_dict_t *indi_light_def_new(
    STR_t name,
    __NULLABLE__ STR_t label,
    indi_state_t value
);

#define indi_light_def_set(def, value) \
            indi_dict_set(def, "$", indi_string_from(indi_state_to_str(value)))

#define indi_light_def_get(def) \
            indi_str_to_state(((indi_string_t *) indi_dict_get(def, "$"))->data)

indi_dict_t *indi_light_def_vector_new(
    STR_t device,
    STR_t name,
    indi_state_t state,
    indi_dict_t *defs[],
    indi_opt_t *opt
);

indi_dict_t *indi_light_set_vector_new(
    indi_dict_t *def_vector
);

/*--------------------------------------------------------------------------------------------------------------------*/

indi_dict_t *indi_switch_def_new(
    STR_t name,
    __NULLABLE__ STR_t label,
    indi_onoff_t value
);

#define indi_switch_def_set(def, value) \
            indi_dict_set(def, "$", indi_string_from(indi_onoff_to_str(value)))

#define indi_switch_def_get(def) \
            indi_str_to_onoff(((indi_string_t *) indi_dict_get(def, "$"))->value)

indi_dict_t *indi_switch_def_vector_new(
    STR_t device,
    STR_t name,
    indi_state_t state,
    indi_perm_t perm,
    indi_rule_t rule,
    indi_dict_t *defs[],
    indi_opt_t *opt
);

indi_dict_t *indi_switch_set_vector_new(
    indi_dict_t *def_vector
);

/*--------------------------------------------------------------------------------------------------------------------*/
/* VALIDATION                                                                                                         */
/*--------------------------------------------------------------------------------------------------------------------*/

bool indi_validation_initialize();

bool indi_validation_finalize();

bool indi_validation_check(
    const indi_xmldoc_t *doc
);

/*--------------------------------------------------------------------------------------------------------------------*/
/* TRANSFORM                                                                                                          */
/*--------------------------------------------------------------------------------------------------------------------*/

indi_object_t *indi_xmldoc_to_object(
    const indi_xmldoc_t *doc,
    bool validate
);

indi_xmldoc_t *indi_object_to_xmldoc(
    const indi_object_t *object,
    bool validate
);

/*--------------------------------------------------------------------------------------------------------------------*/
/* SERVER                                                                                                             */
/*--------------------------------------------------------------------------------------------------------------------*/

int indi_run(
    STR_t url,
    __NULLABLE__ STR_t username,
    __NULLABLE__ STR_t password,
    STR_t client_id,
    indi_list_t *driver_list,
    indi_dict_t *vector_list[],
    bool emit_xml,
    bool validate_xml
);

/*--------------------------------------------------------------------------------------------------------------------*/

#endif /* INDI_BASE_INDI_BASE_H */

/*--------------------------------------------------------------------------------------------------------------------*/

#pragma clang diagnostic pop

/*--------------------------------------------------------------------------------------------------------------------*/
