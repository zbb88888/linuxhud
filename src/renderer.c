#include "linuxhud.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <cairo.h>
#include <pango/pangocairo.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// 未使用参数宏
#define UNUSED(x) ((void)(x))

linuxhud_error_t linuxhud_renderer_init(linuxhud_render_t *render, 
                                        void *buffer, uint32_t width, uint32_t height, uint32_t stride) {
    // 创建 Cairo 表面
    render->cairo_surface = cairo_image_surface_create_for_data(
        (unsigned char *)buffer,
        CAIRO_FORMAT_ARGB32,
        width, height, stride);
    
    if (cairo_surface_status(render->cairo_surface) != CAIRO_STATUS_SUCCESS) {
        return LINUXHUD_ERROR_RENDER;
    }
    
    // 创建 Cairo 上下文
    render->cairo_context = cairo_create(render->cairo_surface);
    if (cairo_status(render->cairo_context) != CAIRO_STATUS_SUCCESS) {
        cairo_surface_destroy(render->cairo_surface);
        return LINUXHUD_ERROR_RENDER;
    }
    
    // 创建 Pango 布局
    render->pango_layout = pango_cairo_create_layout(render->cairo_context);
    if (!render->pango_layout) {
        cairo_destroy(render->cairo_context);
        cairo_surface_destroy(render->cairo_surface);
        return LINUXHUD_ERROR_RENDER;
    }
    
    // 设置默认值
    render->text_color = 0xFFFFFFFF;  // 白色
    render->bg_color = 0x80000000;    // 半透明黑色
    render->font_size = 24;
    render->font_family = "Sans";
    
    // 设置字体
    PangoFontDescription *font_desc = pango_font_description_new();
    pango_font_description_set_family(font_desc, render->font_family);
    pango_font_description_set_size(font_desc, render->font_size * PANGO_SCALE);
    pango_layout_set_font_description(render->pango_layout, font_desc);
    pango_font_description_free(font_desc);
    
    printf("Renderer initialized: %ux%u\n", width, height);
    return LINUXHUD_OK;
}

void linuxhud_renderer_clear(linuxhud_render_t *render, uint32_t width, uint32_t height) {
    cairo_t *cr = render->cairo_context;
    
    // 清除背景
    cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
    cairo_set_source_rgba(cr, 
        ((render->bg_color >> 16) & 0xFF) / 255.0,
        ((render->bg_color >> 8) & 0xFF) / 255.0,
        (render->bg_color & 0xFF) / 255.0,
        ((render->bg_color >> 24) & 0xFF) / 255.0);
    cairo_rectangle(cr, 0, 0, width, height);
    cairo_fill(cr);
}

void linuxhud_renderer_draw_text(linuxhud_render_t *render, const char *text, 
                                 int x, int y, uint32_t width, uint32_t height) {
    UNUSED(height);  // 预留用于未来扩展
    cairo_t *cr = render->cairo_context;
    PangoLayout *layout = render->pango_layout;
    
    // 设置文本
    pango_layout_set_text(layout, text, -1);
    
    // 设置自动换行
    pango_layout_set_width(layout, width * PANGO_SCALE);
    pango_layout_set_wrap(layout, PANGO_WRAP_WORD_CHAR);
    
    // 设置文本颜色
    cairo_set_operator(cr, CAIRO_OPERATOR_OVER);
    cairo_set_source_rgba(cr,
        ((render->text_color >> 16) & 0xFF) / 255.0,
        ((render->text_color >> 8) & 0xFF) / 255.0,
        (render->text_color & 0xFF) / 255.0,
        ((render->text_color >> 24) & 0xFF) / 255.0);
    
    // 绘制文本
    cairo_move_to(cr, x, y);
    pango_cairo_show_layout(cr, layout);
}

void linuxhud_renderer_draw_rect(linuxhud_render_t *render, 
                                 int x, int y, int width, int height, 
                                 uint32_t color, bool fill) {
    cairo_t *cr = render->cairo_context;
    
    cairo_set_operator(cr, CAIRO_OPERATOR_OVER);
    cairo_set_source_rgba(cr,
        ((color >> 16) & 0xFF) / 255.0,
        ((color >> 8) & 0xFF) / 255.0,
        (color & 0xFF) / 255.0,
        ((color >> 24) & 0xFF) / 255.0);
    
    cairo_rectangle(cr, x, y, width, height);
    
    if (fill) {
        cairo_fill(cr);
    } else {
        cairo_stroke(cr);
    }
}

void linuxhud_renderer_draw_rounded_rect(linuxhud_render_t *render,
                                         int x, int y, int width, int height,
                                         int radius, uint32_t color, bool fill) {
    cairo_t *cr = render->cairo_context;
    
    cairo_set_operator(cr, CAIRO_OPERATOR_OVER);
    cairo_set_source_rgba(cr,
        ((color >> 16) & 0xFF) / 255.0,
        ((color >> 8) & 0xFF) / 255.0,
        (color & 0xFF) / 255.0,
        ((color >> 24) & 0xFF) / 255.0);
    
    // 绘制圆角矩形
    cairo_new_sub_path(cr);
    cairo_arc(cr, x + width - radius, y + radius, radius, -M_PI/2, 0);
    cairo_arc(cr, x + width - radius, y + height - radius, radius, 0, M_PI/2);
    cairo_arc(cr, x + radius, y + height - radius, radius, M_PI/2, M_PI);
    cairo_arc(cr, x + radius, y + radius, radius, M_PI, 3*M_PI/2);
    cairo_close_path(cr);
    
    if (fill) {
        cairo_fill(cr);
    } else {
        cairo_stroke(cr);
    }
}

void linuxhud_renderer_set_text_color(linuxhud_render_t *render, uint32_t color) {
    render->text_color = color;
}

void linuxhud_renderer_set_bg_color(linuxhud_render_t *render, uint32_t color) {
    render->bg_color = color;
}

void linuxhud_renderer_set_font(linuxhud_render_t *render, const char *family, uint32_t size) {
    render->font_family = strdup(family);
    render->font_size = size;
    
    PangoFontDescription *font_desc = pango_font_description_new();
    pango_font_description_set_family(font_desc, family);
    pango_font_description_set_size(font_desc, size * PANGO_SCALE);
    pango_layout_set_font_description(render->pango_layout, font_desc);
    pango_font_description_free(font_desc);
}

void linuxhud_renderer_cleanup(linuxhud_render_t *render) {
    if (render->pango_layout) {
        g_object_unref(render->pango_layout);
    }
    
    if (render->cairo_context) {
        cairo_destroy(render->cairo_context);
    }
    
    if (render->cairo_surface) {
        cairo_surface_destroy(render->cairo_surface);
    }
    
    if (render->font_family) {
        free(render->font_family);
    }
}
