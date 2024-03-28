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
            /* no nothing */

/*--------------------------------------------------------------------------------------------------------------------*/

#define buff_t /* */ void *
#define BUFF_t const void *

#define str_t /* */ char *
#define STR_t const char *

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
    INDI_TYPE_NULL    = 100,
    INDI_TYPE_BOOLEAN = 101,
    INDI_TYPE_NUMBER  = 102,
    INDI_TYPE_STRING  = 103,
    INDI_TYPE_DICT    = 104,
    INDI_TYPE_LIST    = 105,

} indi_type_t;

/*--------------------------------------------------------------------------------------------------------------------*/

typedef struct indi_object_s
{
    uint64_t magic;

    enum indi_type_e type;

    __NULLABLE__ struct indi_object_s *parent;

    __NULLABLE__ void (* callback)(const struct indi_object_s *object);

} indi_object_t;

/*--------------------------------------------------------------------------------------------------------------------*/

indi_object_t *indi_object_parse(
    __NULLABLE__ STR_t text
);

void indi_object_free(
    /*-*/ indi_object_t *obj
);

str_t indi_object_to_string(
    const indi_object_t *obj
);

str_t indi_object_to_cstring(
    const indi_object_t *obj
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
    indi_null_t *obj
);

str_t indi_null_to_string(
    indi_null_t *obj
);

/*--------------------------------------------------------------------------------------------------------------------*/
/* NUMBER                                                                                                             */
/*--------------------------------------------------------------------------------------------------------------------*/

typedef struct
{
    indi_object_t base;

    double data;

} indi_number_t;

/*--------------------------------------------------------------------------------------------------------------------*/

indi_number_t *indi_number_new();

void indi_number_free(
    indi_number_t *obj
);

double indi_number_get(
    indi_number_t *obj
);

void indi_number_set(
    indi_number_t *obj,
    double data
);

str_t indi_number_to_string(
    indi_number_t *obj
);

/*--------------------------------------------------------------------------------------------------------------------*/

__INLINE__ indi_number_t *indi_number_from(double data)
{
    indi_number_t *result = indi_number_new();

    indi_number_set(result, data);

    return result;
}

/*--------------------------------------------------------------------------------------------------------------------*/
/* BOOLEAN                                                                                                            */
/*--------------------------------------------------------------------------------------------------------------------*/

typedef struct
{
    indi_object_t base;

    bool data;

} indi_boolean_t;

/*--------------------------------------------------------------------------------------------------------------------*/

indi_boolean_t *indi_boolean_new();

void indi_boolean_free(
    indi_boolean_t *obj
);

bool indi_boolean_get(
    indi_boolean_t *obj
);

void indi_boolean_set(
    indi_boolean_t *obj,
    bool data
);

str_t indi_boolean_to_string(
    indi_boolean_t *obj
);

/*--------------------------------------------------------------------------------------------------------------------*/

__INLINE__ indi_boolean_t *indi_boolean_from(bool data)
{
    indi_boolean_t *result = indi_boolean_new();

    indi_boolean_set(result, data);

    return result;
}

/*--------------------------------------------------------------------------------------------------------------------*/
/* STRING                                                                                                             */
/*--------------------------------------------------------------------------------------------------------------------*/

typedef struct
{
    indi_object_t base;

    STR_t data;

} indi_string_t;

/*--------------------------------------------------------------------------------------------------------------------*/

indi_string_t *indi_string_new();

void indi_string_free(
    indi_string_t *obj
);

STR_t indi_string_get(
    indi_string_t *obj
);

void indi_string_set(
    indi_string_t *obj,
    STR_t data
);

str_t indi_string_to_string(
    indi_string_t *obj
);

str_t indi_string_to_cstring(
    indi_string_t *obj
);

/*--------------------------------------------------------------------------------------------------------------------*/

__INLINE__ indi_string_t *indi_string_from(STR_t data)
{
    indi_string_t *result = indi_string_new();

    indi_string_set(result, data);

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

#define INDI_DICT_ITER(obj) \
                ((indi_dict_iter_t) {0, ((indi_dict_t *) (obj))->base.type, ((indi_dict_t *) (obj))->head})

/*--------------------------------------------------------------------------------------------------------------------*/

indi_dict_t *indi_dict_new();

void indi_dict_free(
    indi_dict_t *obj
);

void indi_dict_clear(
    indi_dict_t *obj
);

void indi_dict_del(
    indi_dict_t *obj,
    STR_t key
);

bool indi_dict_iterate(
    indi_dict_iter_t *iter,
    STR_t *key,
    indi_object_t **obj
);

indi_object_t *indi_dict_get(
    indi_dict_t *obj,
    STR_t key
);

void indi_dict_set(
    indi_dict_t *obj,
    STR_t key,
    buff_t val
);

size_t indi_dict_size(
    indi_dict_t *obj
);

str_t indi_dict_to_string(
    indi_dict_t *obj
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

#define INDI_LIST_ITER(obj) \
                ((indi_list_iter_t) {0, ((indi_list_t *) (obj))->base.type, ((indi_list_t *) (obj))->head})

/*--------------------------------------------------------------------------------------------------------------------*/

indi_list_t *indi_list_new();

void indi_list_free(
    indi_list_t *obj
);

void indi_list_clear(
    indi_list_t *obj
);

void indi_list_del(
    indi_list_t *obj,
    int idx
);

bool indi_list_iterate(
    indi_list_iter_t *iter,
    int *idx,
    indi_object_t **obj
);

indi_object_t *indi_list_get(
    indi_list_t *obj,
    int idx
);

indi_list_t *indi_list_set(
    indi_list_t *obj,
    size_t idx,
    buff_t val
);

size_t indi_list_size(
    indi_list_t *obj
);

str_t indi_list_to_string(
    indi_list_t *obj
);

/*--------------------------------------------------------------------------------------------------------------------*/

#define indi_list_push(obj, val) \
            indi_list_set(obj, -1, val)

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
    INDI_STATE_IDLE = 0,
    INDI_STATE_OK = 1,
    INDI_STATE_BUSY = 2,
    INDI_STATE_ALERT = 3,

} indi_state_t;

__NULLABLE__ STR_t indi_state_to_str(
    indi_state_t state
);

/*--------------------------------------------------------------------------------------------------------------------*/

typedef enum
{
    INDI_PERM_RO = 0,
    INDI_PERM_WO = 1,
    INDI_PERM_RW = 2,

} indi_perm_t;

__NULLABLE__ STR_t indi_perm_to_str(
    indi_perm_t perm
);

/*--------------------------------------------------------------------------------------------------------------------*/

typedef enum
{
    INDI_RULE_ONE_OF_MANY = 0,
    INDI_RULE_AT_MOST_ONE = 1,
    INDI_RULE_ANY_OF_MANY = 2,

} indi_rule_t;

__NULLABLE__ STR_t indi_rule_to_str(
    indi_rule_t rule
);

/*--------------------------------------------------------------------------------------------------------------------*/

typedef enum
{
    INDI_ONOFF_ON = 0,
    INDI_ONOFF_OFF = 1,

} indi_onoff_t;

__NULLABLE__ STR_t indi_onoff_to_str(
    indi_onoff_t onoff
);

/*--------------------------------------------------------------------------------------------------------------------*/

typedef struct
{
    __NULLABLE__ STR_t label;
    __NULLABLE__ STR_t group;
    __NULLABLE__ uint32_t timeout;
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

indi_dict_t *indi_number_vector_new(
    STR_t device,
    STR_t name,
    indi_perm_t perm,
    indi_state_t state,
    indi_dict_t *defs[],
    indi_opt_t *opt
);

/*--------------------------------------------------------------------------------------------------------------------*/

indi_dict_t *indi_text_def_new(
    STR_t name,
    __NULLABLE__ STR_t label,
    STR_t value
);

#define indi_text_def_set(def, value) \
            indi_dict_set(def, "$", indi_string_from(value))

indi_dict_t *indi_text_vector_new(
    STR_t device,
    STR_t name,
    indi_perm_t perm,
    indi_state_t state,
    indi_dict_t *defs[],
    indi_opt_t *opt
);

/*--------------------------------------------------------------------------------------------------------------------*/

indi_dict_t *indi_light_def_new(
    STR_t name,
    __NULLABLE__ STR_t label,
    indi_state_t value
);

#define indi_light_def_set(def, value) \
            indi_dict_set(def, "$", indi_string_from(indi_state_to_str(value)))

indi_dict_t *indi_light_vector_new(
    STR_t device,
    STR_t name,
    indi_state_t state,
    indi_dict_t *defs[],
    indi_opt_t *opt
);

/*--------------------------------------------------------------------------------------------------------------------*/

indi_dict_t *indi_switch_def_new(
    STR_t name,
    __NULLABLE__ STR_t label,
    indi_onoff_t value
);

#define indi_switch_def_set(def, value) \
            indi_dict_set(def, "$", indi_string_from(indi_onoff_to_str(value)))

indi_dict_t *indi_switch_vector_new(
    STR_t device,
    STR_t name,
    indi_state_t state,
    indi_perm_t perm,
    indi_rule_t rule,
    indi_dict_t *defs[],
    indi_opt_t *opt
);

/*--------------------------------------------------------------------------------------------------------------------*/
/* VALIDATION                                                                                                         */
/*--------------------------------------------------------------------------------------------------------------------*/

bool indi_validation_initialize();

bool indi_validation_finalize();

bool indi_validation_check(
    indi_xmldoc_t *doc
);

/*--------------------------------------------------------------------------------------------------------------------*/
/* TRANSFORM                                                                                                          */
/*--------------------------------------------------------------------------------------------------------------------*/

indi_object_t *indi_xmldoc_to_object(
    indi_xmldoc_t *doc,
    bool validate
);

indi_xmldoc_t *indi_object_to_xmldoc(
    indi_object_t *obj,
    bool validate
);

/*--------------------------------------------------------------------------------------------------------------------*/
/* SERVER                                                                                                             */
/*--------------------------------------------------------------------------------------------------------------------*/

int indi_run(
    STR_t url,
    __NULLABLE__ STR_t username,
    __NULLABLE__ STR_t password,
    STR_t client_id
);

/*--------------------------------------------------------------------------------------------------------------------*/

#endif /* INDI_BASE_INDI_BASE_H */

/*--------------------------------------------------------------------------------------------------------------------*/
