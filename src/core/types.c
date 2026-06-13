#include "linuxhud/types.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ========================================================================
 * 错误码描述
 * ======================================================================== */

const char *linuxhud_error_string(linuxhud_error_t error) {
    switch (error) {
        case LINUXHUD_OK:             return "Success";
        case LINUXHUD_ERROR_NOMEM:    return "Out of memory";
        case LINUXHUD_ERROR_INVAL:    return "Invalid argument";
        case LINUXHUD_ERROR_DEVICE:   return "Device error";
        case LINUXHUD_ERROR_RESOURCE: return "Resource error";
        case LINUXHUD_ERROR_PLANE:    return "Plane error";
        case LINUXHUD_ERROR_BUFFER:   return "Buffer error";
        case LINUXHUD_ERROR_RENDER:   return "Render error";
        case LINUXHUD_ERROR_PERMISSION: return "Permission denied";
        case LINUXHUD_ERROR_NOT_FOUND:  return "Not found";
        case LINUXHUD_ERROR_BUSY:       return "Resource busy";
        case LINUXHUD_ERROR_IO:         return "I/O error";
        default:                        return "Unknown error";
    }
}

/* ========================================================================
 * 颜色解析
 * ======================================================================== */

linuxhud_error_t linuxhud_color_parse(const char *str, linuxhud_color_t *out) {
    if (!str || !out) {
        return LINUXHUD_ERROR_INVAL;
    }

    /* 跳过前导 '#' */
    if (str[0] == '#') {
        str++;
    }

    char *endptr = NULL;
    uint32_t val = (uint32_t)strtoul(str, &endptr, 16);

    if (*endptr != '\0') {
        return LINUXHUD_ERROR_INVAL;
    }

    size_t len = strlen(str);
    if (len == 6) {
        /* RRGGBB → 不透明 */
        *out = val | 0xFF000000;
    } else if (len == 8) {
        /* AARRGGBB */
        *out = val;
    } else {
        return LINUXHUD_ERROR_INVAL;
    }

    return LINUXHUD_OK;
}
