#ifndef LINUXHUD_WIDGET_H
#define LINUXHUD_WIDGET_H

#include "types.h"
#include "render.h"

/* ========================================================================
 * Widget 系统
 *
 * 设计理念:
 *   - Widget 是 HUD 上的可视化元素
 *   - 每个 Widget 持有自己的样式和内容
 *   - Widget 通过渲染器绘制到 framebuffer
 *   - 支持组合模式（一个 HUD 可以包含多个 Widget）
 * ======================================================================== */

/* Widget 类型标识 */
typedef enum {
    LINUXHUD_WIDGET_LABEL,        /* 文本标签 */
    LINUXHUD_WIDGET_RECT,         /* 矩形 */
    LINUXHUD_WIDGET_PROGRESS,     /* 进度条 */
    LINUXHUD_WIDGET_GROUP,        /* 容器（包含子 Widget） */
} linuxhud_widget_type_t;

/* Widget 基础结构 (所有 Widget 类型的公共部分) */
typedef struct linuxhud_widget {
    linuxhud_widget_type_t type;
    linuxhud_rect_t        bounds;     /* 位置和大小 */
    bool                   visible;    /* 是否可见 */
    uint8_t                alpha;      /* 全局透明度 0-255 */
    struct linuxhud_widget *next;      /* 兄弟链表 */
} linuxhud_widget_t;

/* ---- Label (文本标签) ---- */

typedef struct {
    linuxhud_widget_t    base;
    char                *text;         /* 文本内容（拥有所有权） */
    linuxhud_text_style_t style;
    linuxhud_color_t     bg_color;     /* 背景色 (TRANSPARENT = 透明) */
    uint32_t             padding;      /* 内边距 */
    uint32_t             border_radius; /* 圆角半径 */
} linuxhud_label_t;

linuxhud_label_t *linuxhud_label_create(const char *text);
void linuxhud_label_set_text(linuxhud_label_t *label, const char *text);
void linuxhud_label_set_color(linuxhud_label_t *label, linuxhud_color_t color);
void linuxhud_label_set_bg(linuxhud_label_t *label, linuxhud_color_t color);
void linuxhud_label_set_font(linuxhud_label_t *label, const linuxhud_font_t *font);
void linuxhud_label_set_align(linuxhud_label_t *label, linuxhud_align_t align);
void linuxhud_label_set_padding(linuxhud_label_t *label, uint32_t padding);
void linuxhud_label_set_radius(linuxhud_label_t *label, uint32_t radius);

/* ---- Rect (矩形) ---- */

typedef struct {
    linuxhud_widget_t  base;
    linuxhud_color_t   fill_color;
    linuxhud_color_t   stroke_color;
    uint32_t           stroke_width;
    uint32_t           border_radius;
} linuxhud_rect_widget_t;

linuxhud_rect_widget_t *linuxhud_rect_create(void);
void linuxhud_rect_set_fill(linuxhud_rect_widget_t *rect, linuxhud_color_t color);
void linuxhud_rect_set_stroke(linuxhud_rect_widget_t *rect, linuxhud_color_t color,
                              uint32_t width);
void linuxhud_rect_set_radius(linuxhud_rect_widget_t *rect, uint32_t radius);

/* ---- Progress (进度条) ---- */

typedef struct {
    linuxhud_widget_t  base;
    float              progress;      /* 0.0 ~ 1.0 */
    linuxhud_color_t   bar_color;     /* 进度条颜色 */
    linuxhud_color_t   track_color;   /* 轨道背景色 */
    linuxhud_color_t   text_color;    /* 百分比文字颜色 (TRANSPARENT = 不显示) */
    uint32_t           border_radius;
} linuxhud_progress_t;

linuxhud_progress_t *linuxhud_progress_create(void);
void linuxhud_progress_set_value(linuxhud_progress_t *prog, float value);
void linuxhud_progress_set_colors(linuxhud_progress_t *prog,
                                  linuxhud_color_t bar,
                                  linuxhud_color_t track);

/* ---- Widget 通用操作 ---- */

/**
 * 设置 Widget 位置和大小
 */
void linuxhud_widget_set_bounds(linuxhud_widget_t *widget, const linuxhud_rect_t *rect);
void linuxhud_widget_set_position(linuxhud_widget_t *widget, int32_t x, int32_t y);
void linuxhud_widget_set_size(linuxhud_widget_t *widget, uint32_t w, uint32_t h);
void linuxhud_widget_set_visible(linuxhud_widget_t *widget, bool visible);

/**
 * 绘制单个 Widget 到渲染器
 * @param widget   Widget 指针
 * @param renderer 渲染器
 */
void linuxhud_widget_draw(linuxhud_widget_t *widget,
                          linuxhud_renderer_t *renderer);

/**
 * 销毁 Widget（及其所有子 Widget）
 */
void linuxhud_widget_destroy(linuxhud_widget_t *widget);

/* ---- Group (容器) ---- */

typedef struct {
    linuxhud_widget_t  base;
    linuxhud_widget_t *children;      /* 子 Widget 链表 */
    linuxhud_color_t   bg_color;      /* 容器背景色 */
    uint32_t           padding;
    uint32_t           border_radius;
    uint32_t           gap;           /* 子元素间距 */
} linuxhud_group_t;

linuxhud_group_t *linuxhud_group_create(void);
void linuxhud_group_add(linuxhud_group_t *group, linuxhud_widget_t *child);
void linuxhud_group_remove(linuxhud_group_t *group, linuxhud_widget_t *child);
void linuxhud_group_set_bg(linuxhud_group_t *group, linuxhud_color_t color);
void linuxhud_group_set_padding(linuxhud_group_t *group, uint32_t padding);
void linuxhud_group_set_gap(linuxhud_group_t *group, uint32_t gap);
void linuxhud_group_set_radius(linuxhud_group_t *group, uint32_t radius);

/**
 * 垂直布局：自动排列子元素
 */
void linuxhud_group_layout_vertical(linuxhud_group_t *group);

#endif /* LINUXHUD_WIDGET_H */
