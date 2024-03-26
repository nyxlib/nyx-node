/*--------------------------------------------------------------------------------------------------------------------*/

#include "../indi_base_internal.h"

/*--------------------------------------------------------------------------------------------------------------------*/

indi_null_t *indi_null_new()
{
    /*----------------------------------------------------------------------------------------------------------------*/

    indi_null_t *obj = indi_memory_alloc(sizeof(indi_null_t));

    /*----------------------------------------------------------------------------------------------------------------*/

    obj->base.magic = INDI_OBJECT_MAGIC;
    obj->base.type = INDI_TYPE_NULL;

    obj->base.parent = NULL;
    obj->base.callback = NULL;

    /*----------------------------------------------------------------------------------------------------------------*/

    return obj;
}

/*--------------------------------------------------------------------------------------------------------------------*/

void indi_null_free(indi_null_t *obj)
{
    indi_memory_free(obj);
}

/*--------------------------------------------------------------------------------------------------------------------*/

str_t indi_null_to_string(__USED__ indi_null_t *obj)
{
    return indi_string_dup("null");
}

/*--------------------------------------------------------------------------------------------------------------------*/
