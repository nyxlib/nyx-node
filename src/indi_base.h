/*--------------------------------------------------------------------------------------------------------------------*/

#ifndef INDI_BASE_INDI_BASE_H
#define INDI_BASE_INDI_BASE_H

/*--------------------------------------------------------------------------------------------------------------------*/

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

/*--------------------------------------------------------------------------------------------------------------------*/

#define __USED__ __attribute__ ((unused))

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

    struct indi_object_s *parent;

    void (* callback)(const struct indi_object_s *object);

} indi_object_t;

/*--------------------------------------------------------------------------------------------------------------------*/

indi_object_t *indi_object_parse(
    STR_t text
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

__USED__
static inline indi_number_t *indi_number_from(double data)
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

__USED__
static inline indi_boolean_t *indi_boolean_from(bool data)
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

    struct indi_string_node_s *head;
    struct indi_string_node_s *tail;

} indi_string_t;

/*--------------------------------------------------------------------------------------------------------------------*/

indi_string_t *indi_string_new();

void indi_string_free(
    indi_string_t *obj
);

void indi_string_clear(
    indi_string_t *obj
);

void indi_string_append_n(
    indi_string_t *obj,
    STR_t args[],
    size_t n
);

void indi_string_notify(
    indi_string_t *obj
);

size_t indi_string_length(
    indi_string_t *obj
);

str_t indi_string_to_string(
    indi_string_t *obj
);

str_t indi_string_to_cstring(
    indi_string_t *obj
);

/*--------------------------------------------------------------------------------------------------------------------*/

#define indi_string_append(obj, ...) ({                                                                                \
                                                                                                                       \
            STR_t args[] = {__VA_ARGS__};                                                                              \
                                                                                                                       \
            indi_string_append_n(obj, args, sizeof(args) / sizeof(STR_t));                                             \
})

/*--------------------------------------------------------------------------------------------------------------------*/

__USED__
static inline indi_string_t *indi_string_from(STR_t data)
{
    indi_string_t *result = indi_string_new();

    indi_string_append(result, data);

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

#define indi_list_push(obj, val) \
            indi_list_set(obj, -1, val)

size_t indi_list_size(
    indi_list_t *obj
);

str_t indi_list_to_string(
    indi_list_t *obj
);

/*--------------------------------------------------------------------------------------------------------------------*/
/* XMLDOC                                                                                                             */
/*--------------------------------------------------------------------------------------------------------------------*/

typedef struct _xmlDoc indi_xmldoc_t;

/*--------------------------------------------------------------------------------------------------------------------*/

indi_xmldoc_t *indi_xmldoc_parse(
    STR_t text
);

void indi_xmldoc_free(
    /*-*/ indi_xmldoc_t *doc
);

str_t indi_xmldoc_to_string(
    const indi_xmldoc_t *doc
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
    /* nullable */ STR_t username,
    /* nullable */ STR_t password,
    STR_t client_id
);

/*--------------------------------------------------------------------------------------------------------------------*/

#endif /* INDI_BASE_INDI_BASE_H */

/*--------------------------------------------------------------------------------------------------------------------*/
