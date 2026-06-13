#include "linuxhud/types.h"
#include <stdio.h>
#include <stdarg.h>

/* ========================================================================
 * 日志系统
 * ======================================================================== */

static linuxhud_log_level_t g_log_level = LINUXHUD_LOG_INFO;
static linuxhud_log_func_t  g_log_callback = NULL;

static const char *level_strings[] = {
    "DEBUG", "INFO", "WARN", "ERROR", "NONE"
};

void linuxhud_log_set_level(linuxhud_log_level_t level) {
    g_log_level = level;
}

void linuxhud_log_set_callback(linuxhud_log_func_t callback) {
    g_log_callback = callback;
}

/* 默认日志输出 */
static void default_log_handler(linuxhud_log_level_t level,
                                const char *file, int line,
                                const char *fmt, va_list args) {
    (void)file;
    (void)line;

    fprintf(stderr, "[LinuxHUD][%s] ", level_strings[level]);
    vfprintf(stderr, fmt, args);
    fprintf(stderr, "\n");
}

void linuxhud_log_impl(linuxhud_log_level_t level,
                       const char *file, int line,
                       const char *fmt, ...) {
    if (level < g_log_level) {
        return;
    }

    va_list args;
    va_start(args, fmt);

    if (g_log_callback) {
        /* 用户自定义回调 */
        g_log_callback(level, file, line, fmt, args);
    } else {
        default_log_handler(level, file, line, fmt, args);
    }

    va_end(args);
}
