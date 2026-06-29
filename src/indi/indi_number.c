/* NyxNode
 * Author: Jérôme ODIER <jerome.odier@lpsc.in2p3.fr>
 * SPDX-License-Identifier: GPL-2.0-only (Mongoose backend) or GPL-3.0+
 */

/*--------------------------------------------------------------------------------------------------------------------*/

#include <stdio.h>

#include "../nyx_node_internal.h"

/*--------------------------------------------------------------------------------------------------------------------*/
/* PROP                                                                                                               */
/*--------------------------------------------------------------------------------------------------------------------*/

static nyx_dict_t *_prop_new(STR_t name, STR_t label, STR_t format, nyx_variant_t min, nyx_variant_t max, nyx_variant_t step, nyx_variant_t value)
{
    if(label == NULL || label[0] == '\0')
    {
        label = name;
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    nyx_dict_t *result = nyx_dict_new();

    /*----------------------------------------------------------------------------------------------------------------*/

    nyx_dict_set_string_unref(result, "<>", "defNumber", false);

    /*----------------------------------------------------------------------------------------------------------------*/

    nyx_dict_set_string_unref(result, "@name", nyx_string_dup(name), true);
    nyx_dict_set_string_unref(result, "@label", nyx_string_dup(label), true);
    nyx_dict_set_string_unref(result, "@format", nyx_string_dup(format), true);

    nyx_dict_set_string_unref(result, "@min", internal_variant_to_string(format, min), true);
    nyx_dict_set_string_unref(result, "@max", internal_variant_to_string(format, max), true);
    nyx_dict_set_string_unref(result, "@step", internal_variant_to_string(format, step), true);

    /*----------------------------------------------------------------------------------------------------------------*/

    nyx_dict_set_string_unref(result, "$", internal_variant_to_string(format, value), true);

    /*----------------------------------------------------------------------------------------------------------------*/

    return result;
}

/*--------------------------------------------------------------------------------------------------------------------*/

nyx_dict_t *nyx_number_prop_new_int(STR_t name, STR_t label, STR_t format, int32_t min, int32_t max, int32_t step, int32_t value)
{
    return _prop_new(name, label, format, NYX_VARIANT_FROM_INT(min), NYX_VARIANT_FROM_INT(max), NYX_VARIANT_FROM_INT(step), NYX_VARIANT_FROM_INT(value));
}

nyx_dict_t *nyx_number_prop_new_uint(STR_t name, STR_t label, STR_t format, uint32_t min, uint32_t max, uint32_t step, uint32_t value)
{
    return _prop_new(name, label, format, NYX_VARIANT_FROM_UINT(min), NYX_VARIANT_FROM_UINT(max), NYX_VARIANT_FROM_UINT(step), NYX_VARIANT_FROM_UINT(value));
}

nyx_dict_t *nyx_number_prop_new_long(STR_t name, STR_t label, STR_t format, int64_t min, int64_t max, int64_t step, int64_t value)
{
    return _prop_new(name, label, format, NYX_VARIANT_FROM_LONG(min), NYX_VARIANT_FROM_LONG(max), NYX_VARIANT_FROM_LONG(step), NYX_VARIANT_FROM_LONG(value));
}

nyx_dict_t *nyx_number_prop_new_ulong(STR_t name, STR_t label, STR_t format, uint64_t min, uint64_t max, uint64_t step, uint64_t value)
{
    return _prop_new(name, label, format, NYX_VARIANT_FROM_ULONG(min), NYX_VARIANT_FROM_ULONG(max), NYX_VARIANT_FROM_ULONG(step), NYX_VARIANT_FROM_ULONG(value));
}

nyx_dict_t *nyx_number_prop_new_double(STR_t name, STR_t label, STR_t format, double min, double max, double step, double value)
{
    return _prop_new(name, label, format, NYX_VARIANT_FROM_DOUBLE(min), NYX_VARIANT_FROM_DOUBLE(max), NYX_VARIANT_FROM_DOUBLE(step), NYX_VARIANT_FROM_DOUBLE(value));
}

/*--------------------------------------------------------------------------------------------------------------------*/
/* PROP SETTER & GETTER                                                                                               */
/*--------------------------------------------------------------------------------------------------------------------*/

static bool _prop_set(const nyx_dict_t *prop, nyx_variant_t value)
{
    STR_t format = nyx_dict_get_string(prop, "@format");

    return nyx_dict_set_string(prop, "$", internal_variant_to_string(format, value), true);
}

/*--------------------------------------------------------------------------------------------------------------------*/

bool nyx_number_prop_set_int(nyx_dict_t *prop, int32_t value)
{
    return _prop_set(prop, NYX_VARIANT_FROM_INT(value));
}

bool nyx_number_prop_set_uint(nyx_dict_t *prop, uint32_t value)
{
    return _prop_set(prop, NYX_VARIANT_FROM_UINT(value));
}

bool nyx_number_prop_set_long(nyx_dict_t *prop, int64_t value)
{
    return _prop_set(prop, NYX_VARIANT_FROM_LONG(value));
}

bool nyx_number_prop_set_ulong(nyx_dict_t *prop, uint64_t value)
{
    return _prop_set(prop, NYX_VARIANT_FROM_ULONG(value));
}

bool nyx_number_prop_set_double(nyx_dict_t *prop, double value)
{
    return _prop_set(prop, NYX_VARIANT_FROM_DOUBLE(value));
}

/*--------------------------------------------------------------------------------------------------------------------*/

static nyx_variant_t _prop_get(const nyx_dict_t *prop)
{
    STR_t format = nyx_dict_get_string(prop, "@format");

    return internal_string_to_variant(format, nyx_dict_get_string(prop, "$"));
}

/*--------------------------------------------------------------------------------------------------------------------*/

int32_t nyx_number_prop_get_int(const nyx_dict_t *prop)
{
    return _prop_get(prop).value._int;
}

uint32_t nyx_number_prop_get_uint(const nyx_dict_t *prop)
{
    return _prop_get(prop).value._uint;
}

int64_t nyx_number_prop_get_long(const nyx_dict_t *prop)
{
    return _prop_get(prop).value._long;
}

uint64_t nyx_number_prop_get_ulong(const nyx_dict_t *prop)
{
    return _prop_get(prop).value._ulong;
}

double nyx_number_prop_get_double(const nyx_dict_t *prop)
{
    return _prop_get(prop).value._double;
}

/*--------------------------------------------------------------------------------------------------------------------*/
/* VECTOR                                                                                                             */
/*--------------------------------------------------------------------------------------------------------------------*/

nyx_dict_t *nyx_number_vector_new(
    STR_t device,
    STR_t name,
    nyx_state_t state,
    nyx_perm_t perm,
    nyx_dict_t *props[],
    const nyx_opts_t *opts
) {
    nyx_dict_t *result = nyx_dict_new();

    nyx_list_t *children = nyx_list_new();

    /*----------------------------------------------------------------------------------------------------------------*/

    nyx_dict_set_string_unref(result, "<>", "defNumberVector", false);

    /*----------------------------------------------------------------------------------------------------------------*/

    nyx_dict_set_string_unref(result, "@client", "unknown", false);
    nyx_dict_set_string_unref(result, "@device", nyx_string_dup(device), true);
    nyx_dict_set_string_unref(result, "@name", nyx_string_dup(name), true);

    nyx_dict_set_string_unref(result, "@state", nyx_state_to_str(state), false);
    nyx_dict_set_string_unref(result, "@perm", nyx_perm_to_str(perm), false);

    /*----------------------------------------------------------------------------------------------------------------*/

    internal_set_opts(result, opts);

    /*----------------------------------------------------------------------------------------------------------------*/

    nyx_dict_set(result, "children", children);

    if(props) for(; *props != NULL; props++)
    {
        nyx_list_push(children, *props);

        nyx_object_unref(*props);
    }

    nyx_object_unref(children);

    /*----------------------------------------------------------------------------------------------------------------*/

    return result;
}

/*--------------------------------------------------------------------------------------------------------------------*/
/* VECTOR                                                                                                             */
/*--------------------------------------------------------------------------------------------------------------------*/

nyx_dict_t *nyx_number_set_vector_new(const nyx_dict_t *vector)
{
    return internal_prop_to_set_vector(vector, "setNumberVector", "oneNumber");
}

/*--------------------------------------------------------------------------------------------------------------------*/
