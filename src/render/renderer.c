#include "linuxhud/render.h"
#include "linuxhud/types.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <cairo.h>
#include <pango/pangocairo.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/* ========================================================================
 * 内部结构
 * ======================================================================== */

struct linuxhud_renderer {
    cairo_surface_t  *surface;
    cairo_t          *cr;
    PangoLayout      *layout;
    uint32_t          width;
    uint32_t          height;
    uint32_t          stride;
};

/* ========================================================================
 * 颜色转换辅助
 * ======================================================================== */

static void set_cairo_source_color(cairo_t *cr, linuxhud_color_t color) {
    cairo_set_source_rgba(cr,
        ((color >> 16) & 0xFF) / 255.0,
        ((color >>  8) & 0xFF) / 255.0,
        ((color      ) & 0xFF) / 255.0,
        ((color >> 24) & 0xFF) / 255.0);
}

/* ========================================================================
 * 公共 API
 * ======================================================================== */

linuxhud_error_t linuxhud_renderer_create(linuxhud_renderer_t **out,
                                          void *pixel_data,
                                          uint32_t width, uint32_t height,
                                          uint32_t stride) {
    if (!out || !pixel_data || width == 0 || height == 0) {
        return LINUXHUD_ERROR_INVAL;
    }

    linuxhud_renderer_t *r = calloc(1, sizeof(*r));
    if (!r) return LINUXHUD_ERROR_NOMEM;

    r->width  = width;
    r->height = height;
    r->stride = stride;

    /* 创建 Cairo 表面 */
    r->surface = cairo_image_surface_create_for_data(
        (unsigned char *)pixel_data,
        CAIRO_FORMAT_ARGB32,
        width, height, stride);

    if (cairo_surface_status(r->surface) != CAIRO_STATUS_SUCCESS) {
        LINUXHUD_ERROR("cairo_image_surface_create_for_data failed");
        free(r);
        return LINUXHUD_ERROR_RENDER;
    }

    /* 创建 Cairo 上下文 */
    r->cr = cairo_create(r->surface);
    if (cairo_status(r->cr) != CAIRO_STATUS_SUCCESS) {
        cairo_surface_destroy(r->surface);
        free(r);
        return LINUXHUD_ERROR_RENDER;
    }

    /* 创建 Pango 布局 */
    r->layout = pango_cairo_create_layout(r->cr);
    if (!r->layout) {
        cairo_destroy(r->cr);
        cairo_surface_destroy(r->surface);
        free(r);
        return LINUXHUD_ERROR_RENDER;
    }

    /* 设置默认字体 */
    PangoFontDescription *desc = pango_font_description_new();
    pango_font_description_set_family(desc, "Sans");
    pango_font_description_set_size(desc, 24 * PANGO_SCALE);
    pango_layout_set_font_description(r->layout, desc);
    pango_font_description_free(desc);

    LINUXHUD_DEBUG("Renderer created: %ux%u, stride=%u", width, height, stride);

    *out = r;
    return LINUXHUD_OK;
}

void linuxhud_renderer_bind(linuxhud_renderer_t *renderer,
                            void *pixel_data,
                            uint32_t width, uint32_t height,
                            uint32_t stride) {
    if (!renderer || !pixel_data) return;

    /* 销毁旧对象 */
    if (renderer->layout)  g_object_unref(renderer->layout);
    if (renderer->cr)      cairo_destroy(renderer->cr);
    if (renderer->surface) cairo_surface_destroy(renderer->surface);

    renderer->width  = width;
    renderer->height = height;
    renderer->stride = stride;

    /* 重建 */
    renderer->surface = cairo_image_surface_create_for_data(
        (unsigned char *)pixel_data,
        CAIRO_FORMAT_ARGB32,
        width, height, stride);

    renderer->cr = cairo_create(renderer->surface);
    renderer->layout = pango_cairo_create_layout(renderer->cr);

    /* 恢复默认字体 */
    PangoFontDescription *desc = pango_font_description_new();
    pango_font_description_set_family(desc, "Sans");
    pango_font_description_set_size(desc, 24 * PANGO_SCALE);
    pango_layout_set_font_description(renderer->layout, desc);
    pango_font_description_free(desc);
}

void linuxhud_renderer_clear(linuxhud_renderer_t *renderer,
                             linuxhud_color_t color) {
    if (!renderer) return;

    cairo_t *cr = renderer->cr;
    cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
    set_cairo_source_color(cr, color);
    cairo_rectangle(cr, 0, 0, renderer->width, renderer->height);
    cairo_fill(cr);
}

/* ---- 图形绘制 ---- */

void linuxhud_renderer_fill_rect(linuxhud_renderer_t *renderer,
                                 const linuxhud_rect_t *rect,
                                 linuxhud_color_t color) {
    if (!renderer || !rect) return;

    cairo_t *cr = renderer->cr;
    cairo_set_operator(cr, CAIRO_OPERATOR_OVER);
    set_cairo_source_color(cr, color);
    cairo_rectangle(cr, rect->x, rect->y, rect->w, rect->h);
    cairo_fill(cr);
}

void linuxhud_renderer_stroke_rect(linuxhud_renderer_t *renderer,
                                   const linuxhud_rect_t *rect,
                                   linuxhud_color_t color, uint32_t line_width) {
    if (!renderer || !rect) return;

    cairo_t *cr = renderer->cr;
    cairo_set_operator(cr, CAIRO_OPERATOR_OVER);
    set_cairo_source_color(cr, color);
    cairo_set_line_width(cr, line_width);
    cairo_rectangle(cr, rect->x, rect->y, rect->w, rect->h);
    cairo_stroke(cr);
}

void linuxhud_renderer_fill_rounded_rect(linuxhud_renderer_t *renderer,
                                         const linuxhud_rect_t *rect,
                                         uint32_t radius,
                                         linuxhud_color_t color) {
    if (!renderer || !rect) return;

    cairo_t *cr = renderer->cr;
    double x = rect->x, y = rect->y;
    double w = rect->w, h = rect->h;
    double r = (double)radius;
    /* 限制半径不超过短边的一半 */
    if (r > w / 2.0) r = w / 2.0;
    if (r > h / 2.0) r = h / 2.0;

    cairo_set_operator(cr, CAIRO_OPERATOR_OVER);
    set_cairo_source_color(cr, color);

    cairo_new_sub_path(cr);
    cairo_arc(cr, x + w - r, y + r,     r, -M_PI / 2, 0);
    cairo_arc(cr, x + w - r, y + h - r, r, 0,          M_PI / 2);
    cairo_arc(cr, x + r,     y + h - r, r, M_PI / 2,   M_PI);
    cairo_arc(cr, x + r,     y + r,     r, M_PI,        3 * M_PI / 2);
    cairo_close_path(cr);
    cairo_fill(cr);
}

void linuxhud_renderer_fill_circle(linuxhud_renderer_t *renderer,
                                   int32_t cx, int32_t cy, uint32_t radius,
                                   linuxhud_color_t color) {
    if (!renderer) return;

    cairo_t *cr = renderer->cr;
    cairo_set_operator(cr, CAIRO_OPERATOR_OVER);
    set_cairo_source_color(cr, color);
    cairo_arc(cr, cx, cy, radius, 0, 2 * M_PI);
    cairo_fill(cr);
}

void linuxhud_renderer_draw_line(linuxhud_renderer_t *renderer,
                                 int32_t x1, int32_t y1,
                                 int32_t x2, int32_t y2,
                                 linuxhud_color_t color, uint32_t line_width) {
    if (!renderer) return;

    cairo_t *cr = renderer->cr;
    cairo_set_operator(cr, CAIRO_OPERATOR_OVER);
    set_cairo_source_color(cr, color);
    cairo_set_line_width(cr, line_width);
    cairo_move_to(cr, x1, y1);
    cairo_line_to(cr, x2, y2);
    cairo_stroke(cr);
}

/* ---- 文本绘制 ---- */

/**
 * 将 linuxhud_font_t 应用到 PangoLayout
 */
static void apply_font(PangoLayout *layout, const linuxhud_font_t *font) {
    PangoFontDescription *desc = pango_font_description_new();

    pango_font_description_set_family(desc, font->family ? font->family : "Sans");
    pango_font_description_set_size(desc, font->size * PANGO_SCALE);

    PangoStyle style = font->italic ? PANGO_STYLE_ITALIC : PANGO_STYLE_NORMAL;
    PangoWeight weight = font->bold ? PANGO_WEIGHT_BOLD : PANGO_WEIGHT_NORMAL;
    pango_font_description_set_style(desc, style);
    pango_font_description_set_weight(desc, weight);

    pango_layout_set_font_description(layout, desc);
    pango_font_description_free(desc);
}

uint32_t linuxhud_renderer_draw_text(linuxhud_renderer_t *renderer,
                                     const char *text,
                                     const linuxhud_rect_t *rect,
                                     const linuxhud_text_style_t *style) {
    if (!renderer || !text || !rect || !style) return 0;

    cairo_t *cr = renderer->cr;
    PangoLayout *layout = renderer->layout;

    /* 设置字体 */
    apply_font(layout, &style->font);

    /* 设置文本和换行 */
    pango_layout_set_text(layout, text, -1);
    pango_layout_set_width(layout, rect->w * PANGO_SCALE);
    pango_layout_set_wrap(layout, PANGO_WRAP_WORD_CHAR);

    /* 对齐 */
    PangoAlignment align = PANGO_ALIGN_LEFT;
    if (style->align == LINUXHUD_ALIGN_CENTER) align = PANGO_ALIGN_CENTER;
    else if (style->align == LINUXHUD_ALIGN_RIGHT) align = PANGO_ALIGN_RIGHT;
    pango_layout_set_alignment(layout, align);

    /* 行间距 */
    if (style->line_spacing > 0) {
        pango_layout_set_line_spacing(layout, style->line_spacing / 100.0f);
    }

    /* 绘制 */
    cairo_set_operator(cr, CAIRO_OPERATOR_OVER);
    set_cairo_source_color(cr, style->color);
    cairo_move_to(cr, rect->x, rect->y);
    pango_cairo_show_layout(cr, layout);

    /* 返回实际占用高度 */
    int text_w, text_h;
    pango_layout_get_pixel_size(layout, &text_w, &text_h);
    return (uint32_t)text_h;
}

void linuxhud_renderer_measure_text(linuxhud_renderer_t *renderer,
                                    const char *text,
                                    uint32_t max_width,
                                    const linuxhud_text_style_t *style,
                                    uint32_t *out_width, uint32_t *out_height) {
    if (!renderer || !text || !style) {
        if (out_width)  *out_width = 0;
        if (out_height) *out_height = 0;
        return;
    }

    PangoLayout *layout = renderer->layout;
    apply_font(layout, &style->font);

    pango_layout_set_text(layout, text, -1);
    pango_layout_set_width(layout, max_width * PANGO_SCALE);
    pango_layout_set_wrap(layout, PANGO_WRAP_WORD_CHAR);

    int w, h;
    pango_layout_get_pixel_size(layout, &w, &h);

    if (out_width)  *out_width  = (uint32_t)w;
    if (out_height) *out_height = (uint32_t)h;
}

void linuxhud_renderer_destroy(linuxhud_renderer_t *renderer) {
    if (!renderer) return;

    if (renderer->layout)  g_object_unref(renderer->layout);
    if (renderer->cr)      cairo_destroy(renderer->cr);
    if (renderer->surface) cairo_surface_destroy(renderer->surface);
    free(renderer);
}
