/* NyxNode
 * Author: Jérôme ODIER <jerome.odier@lpsc.in2p3.fr>
 * SPDX-License-Identifier: GPL-2.0-only (Mongoose backend) or GPL-3.0+
 */

/*--------------------------------------------------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../nyx_node_internal.h"

/*--------------------------------------------------------------------------------------------------------------------*/
/* FORMAT INT                                                                                                         */
/*--------------------------------------------------------------------------------------------------------------------*/

nyx_string_t *nyx_format_int_to_string(nyx_string_t *format, int value)
{
    char buffer[64];

    if(strchr(format->value, 'd') == NULL || snprintf(buffer, sizeof(buffer), format->value, (int) value) == 0) {
        NYX_LOG_ERROR("This function is not compatible with the format `%s`", format);
    }
    else {
        return nyx_string_dynamic_from(buffer);
    }

    return nyx_string_dynamic_from("0");
}

/*--------------------------------------------------------------------------------------------------------------------*/

int nyx_format_string_to_int(nyx_string_t *format, nyx_string_t *value)
{
    char buffer[64];

    if(strchr(format->value, 'd') == NULL || snprintf(buffer, sizeof(buffer), format->value, (int) strtol(value->value, NULL, 10)) == 0) {
        NYX_LOG_ERROR("This function is not compatible with the format `%s`", format);
    }
    else {
        return (int) strtol(buffer, NULL, 10);
    }

    return 0;
}

/*--------------------------------------------------------------------------------------------------------------------*/
/* FORMAT LONG                                                                                                        */
/*--------------------------------------------------------------------------------------------------------------------*/

nyx_string_t *nyx_format_long_to_string(nyx_string_t *format, long value)
{
    char buffer[64];

    if(strchr(format->value, 'l') == NULL || snprintf(buffer, sizeof(buffer), format->value, (long) value) == 0) {
        NYX_LOG_ERROR("This function is not compatible with the format `%s`", format);
    }
    else {
        return nyx_string_dynamic_from(buffer);
    }

    return nyx_string_dynamic_from("0");
}

/*--------------------------------------------------------------------------------------------------------------------*/

long nyx_format_string_to_long(nyx_string_t *format, nyx_string_t *value)
{
    char buffer[64];

    if(strchr(format->value, 'l') == NULL || snprintf(buffer, sizeof(buffer), format->value, (long) strtol(value->value, NULL, 10)) == 0) {
        NYX_LOG_ERROR("This function is not compatible with the format `%s`", format);
    }
    else {
        return (long) strtol(buffer, NULL, 10);
    }

    return 0;
}

/*--------------------------------------------------------------------------------------------------------------------*/
/* FORMAT DOUBLE                                                                                                      */
/*--------------------------------------------------------------------------------------------------------------------*/

nyx_string_t *nyx_format_double_to_string(nyx_string_t *format, double value)
{
    char buffer[64];

    if(strchr(format->value, 'f') == NULL || snprintf(buffer, sizeof(buffer), format->value, (double) value) == 0) {
        NYX_LOG_ERROR("This function is not compatible with the format `%s`", format);
    }
    else {
        return nyx_string_dynamic_from(buffer);
    }

    return nyx_string_dynamic_from("0.0");
}


/*--------------------------------------------------------------------------------------------------------------------*/

double nyx_format_string_to_double(nyx_string_t *format, nyx_string_t *value)
{
    char buffer[64];

    if(strchr(format->value, 'f') == NULL || snprintf(buffer, sizeof(buffer), format->value, (double) strtod(value->value, NULL /**/)) == 0) {
        NYX_LOG_ERROR("This function is not compatible with the format `%s`", format);
    }
    else {
        return (double) strtof(buffer, NULL /**/);
    }

    return 0;
}

/*--------------------------------------------------------------------------------------------------------------------*/
