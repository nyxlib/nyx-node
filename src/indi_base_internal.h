/*--------------------------------------------------------------------------------------------------------------------*/

#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCUnusedMacroInspection"
#pragma ide diagnostic ignored "bugprone-reserved-identifier"
#pragma ide diagnostic ignored "OCUnusedGlobalDeclarationInspection"

/*--------------------------------------------------------------------------------------------------------------------*/

#ifndef INDI_BASE_INDI_BASE_INTERNAL_H
#define INDI_BASE_INDI_BASE_INTERNAL_H

/*--------------------------------------------------------------------------------------------------------------------*/

#include "indi_base.h"

/*--------------------------------------------------------------------------------------------------------------------*/

#define __USED__ __attribute__ ((unused))

/*--------------------------------------------------------------------------------------------------------------------*/
/* OBJECT                                                                                                             */
/*--------------------------------------------------------------------------------------------------------------------*/

void indi_object_notify(
    const indi_object_t *obj
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

void internal_set_opts(indi_dict_t *dict, indi_opt_t *opt);

bool internal_copy_entry(indi_dict_t *dst, indi_dict_t *src, STR_t key);

indi_dict_t *indi_generate_set_message(indi_dict_t *def_vector, STR_t set_tag_name, STR_t one_tag_name);

/*--------------------------------------------------------------------------------------------------------------------*/

#endif /* INDI_BASE_INDI_BASE_INTERNAL_H */

/*--------------------------------------------------------------------------------------------------------------------*/

#pragma clang diagnostic pop

/*--------------------------------------------------------------------------------------------------------------------*/
