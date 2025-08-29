/* NyxNode
 * Author: Jérôme ODIER <jerome.odier@lpsc.in2p3.fr>
 * SPDX-License-Identifier: GPL-2.0-only (Mongoose backend) or GPL-3.0+
 */

/*--------------------------------------------------------------------------------------------------------------------*/

#include <math.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../nyx_node_internal.h"

/*--------------------------------------------------------------------------------------------------------------------*/
/* UTILITIES                                                                                                          */
/*--------------------------------------------------------------------------------------------------------------------*/

static bool _parse_m_format(int *w_out, int *f_out, STR_t s)
{
    /*----------------------------------------------------------------------------------------------------------------*/

    STR_t p = strchr(s, '%');

    if(p == NULL)
    {
        return false;
    }

    p++;

    STR_t q = strchr(p, '%');

    if(q != NULL)
    {
        return false;
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    bool have_w = false;
    int w = 0;

    while(isdigit((unsigned char) *p))
    {
        have_w = true;
        w = (w * 10) + (int) (*p++ - '0');
    }

    if(!have_w) {
        return false;
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    if(*p != '.')
    {
        return false;
    }

    p++;

    /*----------------------------------------------------------------------------------------------------------------*/

    bool have_f = false;
    int f = 0;

    while(isdigit((unsigned char) *p))
    {
        have_f = true;
        f = (f * 10) + (int) (*p++ - '0');
    }

    if(!have_f) {
        return false;
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    if(*p != 'm')
    {
        return false;
    }

    p++;

    if(*p != '\0')
    {
        return false;
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    if(f != 3 && f != 5 && f != 6 && f != 8 && f != 9)
    {
        return false;
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    *w_out = w;
    *f_out = f;

    return true;
}

/*--------------------------------------------------------------------------------------------------------------------*/

static double _parse_m_value(STR_t s)
{
    /*----------------------------------------------------------------------------------------------------------------*/

    STR_t p = s;

    while(*p != '\0' && isspace((unsigned char) *p))
    {
        p++;
    }

    bool neg;

    /**/ if(*p == '-')
    {
        neg = true;
        p++;
    }
    else if(*p == '+')
    {
        neg = false;
        p++;
    }
    else {
        neg = false;
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    str_t end = NULL;
    long deg_l = strtol(p, &end, 10);

    if(p == end) {
        return nan("");
    }
    else {
        p = end;
    }

    int deg_i = (int) deg_l;

    /*----------------------------------------------------------------------------------------------------------------*/

    if(*p != ':')
    {
        return nan("");
    }

    p++;

    /*----------------------------------------------------------------------------------------------------------------*/

    double min_v;

    double sec_v = 0.0;

    STR_t colon2 = strchr(p, ':');

    if(colon2 != NULL)
    {
        /*------------------------------------------------------------------------------------------------------------*/

        str_t end_mm = NULL;
        long mm_l = strtol(p, &end_mm, 10);

        if(p == end_mm) {
            return nan("");
        }
        else {
            p = colon2;
        }
        min_v = (double) mm_l;

        /*------------------------------------------------------------------------------------------------------------*/

        if(p != end_mm)
        {
            return nan("");
        }

        p++;

        /*------------------------------------------------------------------------------------------------------------*/

        str_t end_ss = NULL;
        sec_v = strtod(p, &end_ss);

        if(p == end_ss) {
            return nan("");
        }
        else {
            p = end_ss;
        }

        /*------------------------------------------------------------------------------------------------------------*/
    }
    else
    {
        /*------------------------------------------------------------------------------------------------------------*/

        str_t end_mm = NULL;
        min_v = strtod(p, &end_mm);

        if(p == end_mm) {
            return nan("");
        }
        else {
            p = end_mm;
        }

        /*------------------------------------------------------------------------------------------------------------*/
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    while(*p != '\0' && isspace((unsigned char) *p))
    {
        p++;
    }

    if(*p != '\0')
    {
        return nan("");
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    if(!(min_v >= 0.0 && min_v < 60.0)) {
        return nan("");
    }

    if(!(sec_v >= 0.0 && sec_v < 60.0)) {
        return nan("");
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    double v = (double) deg_i + (min_v / 60.0) + (sec_v / 3600.0);

    return neg ? -v : +v;

    /*----------------------------------------------------------------------------------------------------------------*/
}

/*--------------------------------------------------------------------------------------------------------------------*/

static bool _format_m(str_t dst_str, size_t dst_len, STR_t format, double value)
{
    /*----------------------------------------------------------------------------------------------------------------*/

    int w;
    int f;

    if(!_parse_m_format(&w, &f, format))
    {
        return false;
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    STR_t sign_str = signbit(value) ? "-"
                                    : ""
    ;

    value = fabs(value);

    /*----------------------------------------------------------------------------------------------------------------*/

    int deg_i = (int) floor(value);

    double frac = value - (double) deg_i;

    /*----------------------------------------------------------------------------------------------------------------*/

    char core[64];

    switch(f)
    {
        case 3: /* :mm */
        {
            long min_1 = llround(frac * 60.0);
            int  min_r = (int) min_1;

            if(min_r >= 60) { min_r = 0; deg_i++; }

            snprintf(core, sizeof(core), "%s%d:%02d", sign_str, deg_i, min_r);
        }
            break;

        case 5: /* :mm.m */
        {
            long min_10 = llround(frac * 600.0);
            int  min_r_i = (int) (min_10 / 10);
            int  min_r_d = (int) (min_10 % 10);

            if(min_r_i >= 60) { min_r_i = 0; deg_i++; }

            snprintf(core, sizeof(core), "%s%d:%02d.%01d", sign_str, deg_i, min_r_i, min_r_d);
        }
            break;

        case 6: /* :mm:ss */
        {
            long sec_1 = llround(frac * 3600.0);
            int  min_r = (int) (sec_1 / 60);
            int  sec_r_i = (int) (sec_1 % 60);

            if(min_r >= 60) { min_r = 0; deg_i++; }

            snprintf(core, sizeof(core), "%s%d:%02d:%02d", sign_str, deg_i, min_r, sec_r_i);
        }
            break;

        case 8: /* :mm:ss.s */
        {
            long sec_10 = llround(frac * 36000.0);
            int  min_r   = (int) (sec_10 / 600);
            int  sec_r_i = (int) ((sec_10 % 600) / 10);
            int  sec_r_d = (int) (sec_10 % 10);

            if(min_r >= 60) { min_r = 0; deg_i++; }

            snprintf(core, sizeof(core), "%s%d:%02d:%02d.%01d", sign_str, deg_i, min_r, sec_r_i, sec_r_d);
        }
            break;

        case 9: default: /* :mm:ss.ss */
        {
            long sec_100 = llround(frac * 360000.0);
            int  min_r    = (int) (sec_100 / 6000);
            int  sec_r_i  = (int) ((sec_100 % 6000) / 100);
            int  sec_r_c  = (int) (sec_100 % 100);

            if(min_r >= 60) { min_r = 0; deg_i++; }

            snprintf(core, sizeof(core), "%s%d:%02d:%02d.%02d", sign_str, deg_i, min_r, sec_r_i, sec_r_c);
        }
            break;
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    if(w > 0) {
        if(snprintf(dst_str, dst_len, "%*s", w, core) < 0) {
            return false;
        }
    } else {
        if(snprintf(dst_str, dst_len, "%s", core) < 0) {
            return false;
        }
    }

    /*----------------------------------------------------------------------------------------------------------------*/

    return true;
}

/*--------------------------------------------------------------------------------------------------------------------*/
/* FORMAT INT                                                                                                         */
/*--------------------------------------------------------------------------------------------------------------------*/

nyx_string_t *nyx_format_int_to_string(nyx_string_t *format, int value)
{
    char buffer[64];

    if((strchr(format->value, 'd') == NULL && strchr(format->value, 'u') == NULL) || snprintf(buffer, sizeof(buffer), format->value, value) < 0)
    {
        if((strchr(format->value, 'x') == NULL && strchr(format->value, 'X') == NULL) || snprintf(buffer, sizeof(buffer), format->value, value) < 0)
        {
            NYX_LOG_ERROR("This function is not compatible with the format `%s`", format->value);
        }
        else
        {
            return nyx_string_from_dup(buffer);
        }
    }
    else
    {
        return nyx_string_from_dup(buffer);
    }

    return nyx_string_from_dup("0");
}

/*--------------------------------------------------------------------------------------------------------------------*/

int nyx_format_string_to_int(nyx_string_t *format, nyx_string_t *value)
{
    if(strchr(format->value, 'd') == NULL && strchr(format->value, 'u') == NULL)
    {
        if(strchr(format->value, 'x') == NULL && strchr(format->value, 'X') == NULL)
        {
            NYX_LOG_ERROR("This function is not compatible with the format `%s`", format->value);
        }
        else
        {
            return (int) strtol(value->value, NULL, 16);
        }
    }
    else
    {
        return (int) strtol(value->value, NULL, 10);
    }

    return 0;
}

/*--------------------------------------------------------------------------------------------------------------------*/
/* FORMAT LONG                                                                                                        */
/*--------------------------------------------------------------------------------------------------------------------*/

nyx_string_t *nyx_format_long_to_string(nyx_string_t *format, long value)
{
    char buffer[64];

    if(strchr(format->value, 'l') == NULL || snprintf(buffer, sizeof(buffer), format->value, (long) value) < 0)
    {
        NYX_LOG_ERROR("This function is not compatible with the format `%s`", format->value);
    }
    else
    {
        return nyx_string_from_dup(buffer);
    }

    return nyx_string_from_dup("0");
}

/*--------------------------------------------------------------------------------------------------------------------*/

long nyx_format_string_to_long(nyx_string_t *format, nyx_string_t *value)
{
    if(strchr(format->value, 'l') == NULL)
    {
        NYX_LOG_ERROR("This function is not compatible with the format `%s`", format->value);
    }
    else
    {
        return (long) strtol(value->value, NULL, 10);
    }

    return 0;
}

/*--------------------------------------------------------------------------------------------------------------------*/
/* FORMAT DOUBLE                                                                                                      */
/*--------------------------------------------------------------------------------------------------------------------*/

nyx_string_t *nyx_format_double_to_string(nyx_string_t *format, double value)
{
    char buffer[64];

    if((strchr(format->value, 'f') == NULL && strchr(format->value, 'e') == NULL && strchr(format->value, 'g') == NULL) || snprintf(buffer, sizeof(buffer), format->value, (double) value) < 0)
    {
        if(!_format_m(buffer, sizeof(buffer), format->value, value))
        {
            NYX_LOG_ERROR("This function is not compatible with the format `%s`", format->value);
        }
        else
        {
            return nyx_string_from_dup(buffer);
        }
    }
    else
    {
        return nyx_string_from_dup(buffer);
    }

    return nyx_string_from_dup("0.0");
}

/*--------------------------------------------------------------------------------------------------------------------*/

double nyx_format_string_to_double(nyx_string_t *format, nyx_string_t *value)
{
    if(strchr(format->value, 'f') == NULL && strchr(format->value, 'e') == NULL && strchr(format->value, 'g') == NULL)
    {
        int w, f;

        if(!_parse_m_format(&w, &f, format->value))
        {
            NYX_LOG_ERROR("This function is not compatible with the format `%s`", format->value);
        }
        else
        {
            return (double) _parse_m_value(value->value);
        }
    }
    else
    {
        return (double) strtod(value->value, NULL);
    }

    return 0;
}

/*--------------------------------------------------------------------------------------------------------------------*/
