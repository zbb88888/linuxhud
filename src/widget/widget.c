#include "linuxhud/widget.h"
#include "linuxhud/types.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ========================================================================
 * Widget 通用操作
 * ======================================================================== */

void linuxhud_widget_set_bounds(linuxhud_widget_t *widget, const linuxhud_rect_t *rect) {
    if (!widget || !rect) return;
    widget->bounds = *rect;
}

void linuxhud_widget_set_position(linuxhud_widget_t *widget, int32_t x, int32_t y) {
    if (!widget) return;
    widget->bounds.x = x;
    widget->bounds.y = y;
}

void linuxhud_widget_set_size(linuxhud_widget_t *widget, uint32_t w, uint32_t h) {
    if (!widget) return;
    widget->bounds.w = w;
    widget->bounds.h = h;
}

void linuxhud_widget_set_visible(linuxhud_widget_t *widget, bool visible) {
    if (widget) widget->visible = visible;
}

/* ========================================================================
 * Label
 * ======================================================================== */

linuxhud_label_t *linuxhud_label_create(const char *text) {
    linuxhud_label_t *label = calloc(1, sizeof(*label));
    if (!label) return NULL;

    label->base.type    = LINUXHUD_WIDGET_LABEL;
    label->base.visible = true;
    label->base.alpha   = 0xFF;

    label->text = text ? strdup(text) : strdup("");
    label->style.color = LINUXHUD_COLOR_WHITE;
    label->style.font.family = NULL;  /* 使用渲染器默认 */
    label->style.font.size = 24;
    label->style.align = LINUXHUD_ALIGN_LEFT;
    label->bg_color = LINUXHUD_COLOR_TRANSPARENT;
    label->padding = 4;
    label->border_radius = 0;

    return label;
}

void linuxhud_label_set_text(linuxhud_label_t *label, const char *text) {
    if (!label) return;
    free(label->text);
    label->text = text ? strdup(text) : strdup("");
}

void linuxhud_label_set_color(linuxhud_label_t *label, linuxhud_color_t color) {
    if (label) label->style.color = color;
}

void linuxhud_label_set_bg(linuxhud_label_t *label, linuxhud_color_t color) {
    if (label) label->bg_color = color;
}

void linuxhud_label_set_font(linuxhud_label_t *label, const linuxhud_font_t *font) {
    if (!label || !font) return;
    label->style.font = *font;
}

void linuxhud_label_set_align(linuxhud_label_t *label, linuxhud_align_t align) {
    if (label) label->style.align = align;
}

void linuxhud_label_set_padding(linuxhud_label_t *label, uint32_t padding) {
    if (label) label->padding = padding;
}

void linuxhud_label_set_radius(linuxhud_label_t *label, uint32_t radius) {
    if (label) label->border_radius = radius;
}

static void label_draw(linuxhud_label_t *label, linuxhud_renderer_t *renderer) {
    /* 背景 */
    if (label->bg_color != LINUXHUD_COLOR_TRANSPARENT) {
        if (label->border_radius > 0) {
            linuxhud_renderer_fill_rounded_rect(renderer, &label->base.bounds,
                                                label->border_radius, label->bg_color);
        } else {
            linuxhud_renderer_fill_rect(renderer, &label->base.bounds, label->bg_color);
        }
    }

    /* 文本 */
    linuxhud_rect_t text_rect = {
        .x = label->base.bounds.x + (int32_t)label->padding,
        .y = label->base.bounds.y + (int32_t)label->padding,
        .w = label->base.bounds.w - 2 * label->padding,
        .h = label->base.bounds.h - 2 * label->padding,
    };
    linuxhud_renderer_draw_text(renderer, label->text, &text_rect, &label->style);
}

/* ========================================================================
 * Rect Widget
 * ======================================================================== */

linuxhud_rect_widget_t *linuxhud_rect_create(void) {
    linuxhud_rect_widget_t *rect = calloc(1, sizeof(*rect));
    if (!rect) return NULL;

    rect->base.type    = LINUXHUD_WIDGET_RECT;
    rect->base.visible = true;
    rect->base.alpha   = 0xFF;
    rect->fill_color   = LINUXHUD_COLOR_WHITE;
    rect->stroke_color = LINUXHUD_COLOR_TRANSPARENT;
    rect->stroke_width = 1;
    rect->border_radius = 0;

    return rect;
}

void linuxhud_rect_set_fill(linuxhud_rect_widget_t *rect, linuxhud_color_t color) {
    if (rect) rect->fill_color = color;
}

void linuxhud_rect_set_stroke(linuxhud_rect_widget_t *rect, linuxhud_color_t color,
                              uint32_t width) {
    if (rect) {
        rect->stroke_color = color;
        rect->stroke_width = width;
    }
}

void linuxhud_rect_set_radius(linuxhud_rect_widget_t *rect, uint32_t radius) {
    if (rect) rect->border_radius = radius;
}

static void rect_draw(linuxhud_rect_widget_t *rect, linuxhud_renderer_t *renderer) {
    if (rect->fill_color != LINUXHUD_COLOR_TRANSPARENT) {
        if (rect->border_radius > 0) {
            linuxhud_renderer_fill_rounded_rect(renderer, &rect->base.bounds,
                                                rect->border_radius, rect->fill_color);
        } else {
            linuxhud_renderer_fill_rect(renderer, &rect->base.bounds, rect->fill_color);
        }
    }

    if (rect->stroke_color != LINUXHUD_COLOR_TRANSPARENT) {
        linuxhud_renderer_stroke_rect(renderer, &rect->base.bounds,
                                      rect->stroke_color, rect->stroke_width);
    }
}

/* ========================================================================
 * Progress Bar
 * ======================================================================== */

linuxhud_progress_t *linuxhud_progress_create(void) {
    linuxhud_progress_t *prog = calloc(1, sizeof(*prog));
    if (!prog) return NULL;

    prog->base.type    = LINUXHUD_WIDGET_PROGRESS;
    prog->base.visible = true;
    prog->base.alpha   = 0xFF;
    prog->progress     = 0.0f;
    prog->bar_color    = LINUXHUD_COLOR_GREEN;
    prog->track_color  = linuxhud_color_rgb(0x40, 0x40, 0x40);
    prog->text_color   = LINUXHUD_COLOR_WHITE;
    prog->border_radius = 4;

    return prog;
}

void linuxhud_progress_set_value(linuxhud_progress_t *prog, float value) {
    if (!prog) return;
    if (value < 0.0f) value = 0.0f;
    if (value > 1.0f) value = 1.0f;
    prog->progress = value;
}

void linuxhud_progress_set_colors(linuxhud_progress_t *prog,
                                  linuxhud_color_t bar,
                                  linuxhud_color_t track) {
    if (!prog) return;
    prog->bar_color   = bar;
    prog->track_color = track;
}

static void progress_draw(linuxhud_progress_t *prog, linuxhud_renderer_t *renderer) {
    linuxhud_rect_t *b = &prog->base.bounds;
    uint32_t r = prog->border_radius;

    /* 轨道背景 */
    if (r > 0) {
        linuxhud_renderer_fill_rounded_rect(renderer, b, r, prog->track_color);
    } else {
        linuxhud_renderer_fill_rect(renderer, b, prog->track_color);
    }

    /* 进度条 */
    uint32_t bar_w = (uint32_t)(b->w * prog->progress);
    if (bar_w > 0) {
        linuxhud_rect_t bar_rect = { b->x, b->y, bar_w, b->h };
        if (r > 0) {
            linuxhud_renderer_fill_rounded_rect(renderer, &bar_rect, r, prog->bar_color);
        } else {
            linuxhud_renderer_fill_rect(renderer, &bar_rect, prog->bar_color);
        }
    }

    /* 百分比文字 */
    if (prog->text_color != LINUXHUD_COLOR_TRANSPARENT) {
        char buf[8];
        snprintf(buf, sizeof(buf), "%d%%", (int)(prog->progress * 100));

        linuxhud_text_style_t style = {
            .color = prog->text_color,
            .font  = { .family = "Mono", .size = b->h * 60 / 100 },
            .align = LINUXHUD_ALIGN_CENTER,
        };
        linuxhud_renderer_draw_text(renderer, buf, b, &style);
    }
}

/* ========================================================================
 * Group (容器)
 * ======================================================================== */

linuxhud_group_t *linuxhud_group_create(void) {
    linuxhud_group_t *group = calloc(1, sizeof(*group));
    if (!group) return NULL;

    group->base.type    = LINUXHUD_WIDGET_GROUP;
    group->base.visible = true;
    group->base.alpha   = 0xFF;
    group->bg_color     = LINUXHUD_COLOR_TRANSPARENT;
    group->padding      = 0;
    group->border_radius = 0;
    group->gap          = 4;
    group->children     = NULL;

    return group;
}

void linuxhud_group_add(linuxhud_group_t *group, linuxhud_widget_t *child) {
    if (!group || !child) return;

    /* 添加到链表末尾 */
    child->next = NULL;
    if (!group->children) {
        group->children = child;
    } else {
        linuxhud_widget_t *tail = group->children;
        while (tail->next) tail = tail->next;
        tail->next = child;
    }
}

void linuxhud_group_remove(linuxhud_group_t *group, linuxhud_widget_t *child) {
    if (!group || !child) return;

    if (group->children == child) {
        group->children = child->next;
        child->next = NULL;
        return;
    }

    linuxhud_widget_t *prev = group->children;
    while (prev && prev->next != child) {
        prev = prev->next;
    }
    if (prev) {
        prev->next = child->next;
        child->next = NULL;
    }
}

void linuxhud_group_set_bg(linuxhud_group_t *group, linuxhud_color_t color) {
    if (group) group->bg_color = color;
}

void linuxhud_group_set_padding(linuxhud_group_t *group, uint32_t padding) {
    if (group) group->padding = padding;
}

void linuxhud_group_set_gap(linuxhud_group_t *group, uint32_t gap) {
    if (group) group->gap = gap;
}

void linuxhud_group_set_radius(linuxhud_group_t *group, uint32_t radius) {
    if (group) group->border_radius = radius;
}

void linuxhud_group_layout_vertical(linuxhud_group_t *group) {
    if (!group) return;

    int32_t y = group->base.bounds.y + (int32_t)group->padding;

    for (linuxhud_widget_t *child = group->children; child; child = child->next) {
        child->bounds.x = group->base.bounds.x + (int32_t)group->padding;
        child->bounds.y = y;
        /* 宽度继承容器 */
        if (child->bounds.w == 0) {
            child->bounds.w = group->base.bounds.w - 2 * group->padding;
        }
        y += child->bounds.h + group->gap;
    }
}

static void group_draw(linuxhud_group_t *group, linuxhud_renderer_t *renderer) {
    /* 背景 */
    if (group->bg_color != LINUXHUD_COLOR_TRANSPARENT) {
        if (group->border_radius > 0) {
            linuxhud_renderer_fill_rounded_rect(renderer, &group->base.bounds,
                                                group->border_radius, group->bg_color);
        } else {
            linuxhud_renderer_fill_rect(renderer, &group->base.bounds, group->bg_color);
        }
    }

    /* 子元素 */
    for (linuxhud_widget_t *child = group->children; child; child = child->next) {
        linuxhud_widget_draw(child, renderer);
    }
}

/* ========================================================================
 * 统一分发
 * ======================================================================== */

void linuxhud_widget_draw(linuxhud_widget_t *widget,
                          linuxhud_renderer_t *renderer) {
    if (!widget || !widget->visible || !renderer) return;

    switch (widget->type) {
        case LINUXHUD_WIDGET_LABEL:
            label_draw((linuxhud_label_t *)widget, renderer);
            break;
        case LINUXHUD_WIDGET_RECT:
            rect_draw((linuxhud_rect_widget_t *)widget, renderer);
            break;
        case LINUXHUD_WIDGET_PROGRESS:
            progress_draw((linuxhud_progress_t *)widget, renderer);
            break;
        case LINUXHUD_WIDGET_GROUP:
            group_draw((linuxhud_group_t *)widget, renderer);
            break;
    }
}

void linuxhud_widget_destroy(linuxhud_widget_t *widget) {
    if (!widget) return;

    /* 如果是 Group，先销毁子元素 */
    if (widget->type == LINUXHUD_WIDGET_GROUP) {
        linuxhud_group_t *group = (linuxhud_group_t *)widget;
        linuxhud_widget_t *child = group->children;
        while (child) {
            linuxhud_widget_t *next = child->next;
            linuxhud_widget_destroy(child);
            child = next;
        }
    }

    /* 如果是 Label，释放文本 */
    if (widget->type == LINUXHUD_WIDGET_LABEL) {
        linuxhud_label_t *label = (linuxhud_label_t *)widget;
        free(label->text);
    }

    free(widget);
}
