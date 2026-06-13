#ifndef LINUXHUD_TYPES_H
#define LINUXHUD_TYPES_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/* ========================================================================
 * 版本信息
 * ======================================================================== */

#define LINUXHUD_VERSION_MAJOR 0
#define LINUXHUD_VERSION_MINOR 2
#define LINUXHUD_VERSION_PATCH 0
#define LINUXHUD_VERSION_STRING "0.2.0"

/* ========================================================================
 * 错误码
 * ======================================================================== */

typedef enum {
    LINUXHUD_OK = 0,              /* 成功 */
    LINUXHUD_ERROR_NOMEM,         /* 内存分配失败 */
    LINUXHUD_ERROR_INVAL,         /* 无效参数 */
    LINUXHUD_ERROR_DEVICE,        /* 设备打开/操作失败 */
    LINUXHUD_ERROR_RESOURCE,      /* DRM 资源获取失败 */
    LINUXHUD_ERROR_PLANE,         /* Plane 分配失败 */
    LINUXHUD_ERROR_BUFFER,        /* Buffer 创建/操作失败 */
    LINUXHUD_ERROR_RENDER,        /* 渲染失败 */
    LINUXHUD_ERROR_PERMISSION,    /* 权限不足 */
    LINUXHUD_ERROR_NOT_FOUND,     /* 资源未找到 */
    LINUXHUD_ERROR_BUSY,          /* 资源忙 */
    LINUXHUD_ERROR_IO,            /* I/O 错误 */
} linuxhud_error_t;

/**
 * 获取错误码的可读描述
 * @param error 错误码
 * @return 静态字符串，无需释放
 */
const char *linuxhud_error_string(linuxhud_error_t error);

/* ========================================================================
 * 日志
 * ======================================================================== */

typedef enum {
    LINUXHUD_LOG_DEBUG = 0,
    LINUXHUD_LOG_INFO,
    LINUXHUD_LOG_WARN,
    LINUXHUD_LOG_ERROR,
    LINUXHUD_LOG_NONE,            /* 关闭日志 */
} linuxhud_log_level_t;

typedef void (*linuxhud_log_func_t)(linuxhud_log_level_t level,
                                    const char *file, int line,
                                    const char *fmt, ...);

/**
 * 设置全局日志级别和回调
 * @param level   最低日志级别
 * @param callback 日志回调函数（NULL 则使用默认 stderr 输出）
 */
void linuxhud_log_set_level(linuxhud_log_level_t level);
void linuxhud_log_set_callback(linuxhud_log_func_t callback);

/* 内部日志宏，不要直接调用 */
void linuxhud_log_impl(linuxhud_log_level_t level,
                       const char *file, int line,
                       const char *fmt, ...)
    __attribute__((format(printf, 4, 5)));

#define LINUXHUD_LOG(level, ...) \
    linuxhud_log_impl((level), __FILE__, __LINE__, __VA_ARGS__)

#define LINUXHUD_DEBUG(...) LINUXHUD_LOG(LINUXHUD_LOG_DEBUG, __VA_ARGS__)
#define LINUXHUD_INFO(...)  LINUXHUD_LOG(LINUXHUD_LOG_INFO,  __VA_ARGS__)
#define LINUXHUD_WARN(...)  LINUXHUD_LOG(LINUXHUD_LOG_WARN,  __VA_ARGS__)
#define LINUXHUD_ERROR(...) LINUXHUD_LOG(LINUXHUD_LOG_ERROR, __VA_ARGS__)

/* ========================================================================
 * 颜色 (ARGB8888)
 * ======================================================================== */

typedef uint32_t linuxhud_color_t;

/* 从 RGBA 分量构造颜色 */
static inline linuxhud_color_t linuxhud_color_rgba(uint8_t r, uint8_t g,
                                                    uint8_t b, uint8_t a) {
    return ((uint32_t)a << 24) | ((uint32_t)r << 16) |
           ((uint32_t)g << 8)  | (uint32_t)b;
}

/* 从 RGB 构造不透明颜色 */
static inline linuxhud_color_t linuxhud_color_rgb(uint8_t r, uint8_t g,
                                                   uint8_t b) {
    return linuxhud_color_rgba(r, g, b, 0xFF);
}

#define LINUXHUD_COLOR_WHITE   linuxhud_color_rgb(0xFF, 0xFF, 0xFF)
#define LINUXHUD_COLOR_BLACK   linuxhud_color_rgb(0x00, 0x00, 0x00)
#define LINUXHUD_COLOR_RED     linuxhud_color_rgb(0xFF, 0x00, 0x00)
#define LINUXHUD_COLOR_GREEN   linuxhud_color_rgb(0x00, 0xFF, 0x00)
#define LINUXHUD_COLOR_BLUE    linuxhud_color_rgb(0x00, 0x00, 0xFF)
#define LINUXHUD_COLOR_TRANSPARENT 0x00000000

/* 从十六进制字符串解析颜色，如 "FF0000" 或 "FFFF0000" */
linuxhud_error_t linuxhud_color_parse(const char *str, linuxhud_color_t *out);

/* ========================================================================
 * 矩形
 * ======================================================================== */

typedef struct {
    int32_t  x;
    int32_t  y;
    uint32_t w;
    uint32_t h;
} linuxhud_rect_t;

/* ========================================================================
 * 未使用参数
 * ======================================================================== */

#define LINUXHUD_UNUSED(x) ((void)(x))

#endif /* LINUXHUD_TYPES_H */
