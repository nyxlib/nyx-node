/*--------------------------------------------------------------------------------------------------------------------*/

#include <math.h>
#include <stdio.h>

#include "../indi_base_internal.h"

/*--------------------------------------------------------------------------------------------------------------------*/

indi_number_t *indi_number_new()
{
    /*----------------------------------------------------------------------------------------------------------------*/

    indi_number_t *obj = indi_memory_alloc(sizeof(indi_number_t));

    /*----------------------------------------------------------------------------------------------------------------*/
    
    obj->base.magic = INDI_OBJECT_MAGIC;
    obj->base.type = INDI_TYPE_NUMBER;

    obj->base.parent = NULL;
    obj->base.out_callback = NULL;

    /*----------------------------------------------------------------------------------------------------------------*/

    obj->data = 0.0;

    /*----------------------------------------------------------------------------------------------------------------*/

    return obj;
}

/*--------------------------------------------------------------------------------------------------------------------*/

void indi_number_free(indi_number_t *obj)
{
    indi_memory_free(obj);
}

/*--------------------------------------------------------------------------------------------------------------------*/

double indi_number_get(indi_number_t *obj)
{
    return obj->data;
}

/*--------------------------------------------------------------------------------------------------------------------*/

void indi_number_set(indi_number_t *obj, double data)
{
    if(isnan(data))
    {
        fprintf(stderr, "NaN number not allowed in `indi_number_set`\n");
        fflush(stderr);
        return;
    }

    if(obj->data != data)
    {
        obj->data = data;

        indi_object_notify(&obj->base);
    }
}

/*--------------------------------------------------------------------------------------------------------------------*/

str_t indi_number_to_string(indi_number_t *obj)
{
    return indi_double_dup(obj->data);
}

/*--------------------------------------------------------------------------------------------------------------------*/
