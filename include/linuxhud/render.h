#ifndef LINUXHUD_RENDER_H
#define LINUXHUD_RENDER_H

#include "types.h"

/* ========================================================================
 * 渲染器
 *
 * 职责:
 *   - 在 framebuffer 像素数据上进行 2D 绘制
 *   - 文本渲染 (Pango)
 *   - 基础图形 (矩形、圆角矩形、线条)
 *
 * 特点:
 *   - 渲染器不拥有 framebuffer，只操作传入的像素缓冲区
 *   - 支持创建多个独立的渲染上下文
 * ======================================================================== */

/* 渲染器句柄 (不透明类型) */
typedef struct linuxhud_renderer linuxhud_renderer_t;

/* 字体配置 */
typedef struct {
    const char *family;           /* 字体族，如 "Sans", "Mono"，NULL 使用默认 */
    uint32_t    size;             /* 字号（像素） */
    bool        bold;
    bool        italic;
} linuxhud_font_t;

/* 文本对齐 */
typedef enum {
    LINUXHUD_ALIGN_LEFT = 0,
    LINUXHUD_ALIGN_CENTER,
    LINUXHUD_ALIGN_RIGHT,
} linuxhud_align_t;

/* 文本样式 */
typedef struct {
    linuxhud_color_t  color;      /* 文本颜色 */
    linuxhud_font_t   font;       /* 字体 */
    linuxhud_align_t  align;      /* 对齐方式 */
    uint32_t          line_spacing; /* 行间距倍数 (100 = 1.0x) */
} linuxhud_text_style_t;

/**
 * 创建渲染器
 * @param[out] out     返回渲染器句柄
 * @param pixel_data   像素缓冲区指针（由 linuxhud_fb_get_data() 获取）
 * @param width        缓冲区宽度
 * @param height       缓冲区高度
 * @param stride       每行字节数
 * @return 错误码
 */
linuxhud_error_t linuxhud_renderer_create(linuxhud_renderer_t **out,
                                          void *pixel_data,
                                          uint32_t width, uint32_t height,
                                          uint32_t stride);

/**
 * 将渲染器绑定到新的像素缓冲区（用于 buffer 切换时复用渲染器）
 * @param renderer     渲染器句柄
 * @param pixel_data   新的像素缓冲区
 * @param width        新宽度
 * @param height       新高度
 * @param stride       新 stride
 */
void linuxhud_renderer_bind(linuxhud_renderer_t *renderer,
                            void *pixel_data,
                            uint32_t width, uint32_t height,
                            uint32_t stride);

/**
 * 清除整个画布为指定颜色
 */
void linuxhud_renderer_clear(linuxhud_renderer_t *renderer,
                             linuxhud_color_t color);

/* ---- 图形绘制 ---- */

/**
 * 填充矩形
 */
void linuxhud_renderer_fill_rect(linuxhud_renderer_t *renderer,
                                 const linuxhud_rect_t *rect,
                                 linuxhud_color_t color);

/**
 * 描边矩形
 */
void linuxhud_renderer_stroke_rect(linuxhud_renderer_t *renderer,
                                   const linuxhud_rect_t *rect,
                                   linuxhud_color_t color, uint32_t line_width);

/**
 * 填充圆角矩形
 */
void linuxhud_renderer_fill_rounded_rect(linuxhud_renderer_t *renderer,
                                         const linuxhud_rect_t *rect,
                                         uint32_t radius,
                                         linuxhud_color_t color);

/**
 * 填充圆形
 */
void linuxhud_renderer_fill_circle(linuxhud_renderer_t *renderer,
                                   int32_t cx, int32_t cy, uint32_t radius,
                                   linuxhud_color_t color);

/**
 * 画线
 */
void linuxhud_renderer_draw_line(linuxhud_renderer_t *renderer,
                                 int32_t x1, int32_t y1,
                                 int32_t x2, int32_t y2,
                                 linuxhud_color_t color, uint32_t line_width);

/* ---- 文本绘制 ---- */

/**
 * 绘制文本（自动换行）
 * @param renderer  渲染器
 * @param text      UTF-8 文本
 * @param rect      绘制区域
 * @param style     文本样式
 * @return 实际占用的高度（像素）
 */
uint32_t linuxhud_renderer_draw_text(linuxhud_renderer_t *renderer,
                                     const char *text,
                                     const linuxhud_rect_t *rect,
                                     const linuxhud_text_style_t *style);

/**
 * 测量文本在给定区域内的实际占用尺寸（不绘制）
 */
void linuxhud_renderer_measure_text(linuxhud_renderer_t *renderer,
                                    const char *text,
                                    uint32_t max_width,
                                    const linuxhud_text_style_t *style,
                                    uint32_t *out_width, uint32_t *out_height);

/**
 * 销毁渲染器
 */
void linuxhud_renderer_destroy(linuxhud_renderer_t *renderer);

#endif /* LINUXHUD_RENDER_H */
