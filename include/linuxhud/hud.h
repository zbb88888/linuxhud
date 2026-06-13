#ifndef LINUXHUD_HUD_H
#define LINUXHUD_HUD_H

#include "types.h"
#include "drm.h"
#include "render.h"
#include "widget.h"

/* ========================================================================
 * HUD 主对象
 *
 * 这是用户直接使用的顶层 API。
 *
 * 生命周期:
 *   linuxhud_create() → linuxhud_show() → linuxhud_update() → linuxhud_destroy()
 *
 * 线程安全:
 *   - 单个 HUD 实例不是线程安全的
 *   - 不同 HUD 实例可以并行使用
 * ======================================================================== */

/* HUD 句柄 (不透明类型) */
typedef struct linuxhud linuxhud_t;

/* HUD 配置 */
typedef struct {
    /* 位置 */
    int32_t  x;
    int32_t  y;

    /* 尺寸 */
    uint32_t width;
    uint32_t height;

    /* 外观 */
    uint8_t        alpha;          /* 全局透明度 0-255 */
    linuxhud_color_t bg_color;     /* 背景色 */

    /* DRM 配置（NULL 使用默认） */
    const linuxhud_drm_config_t *drm_config;
} linuxhud_config_t;

/**
 * 用默认值填充配置
 */
void linuxhud_config_defaults(linuxhud_config_t *config);

/**
 * 创建 HUD 实例
 * @param[out] out    返回 HUD 句柄
 * @param[in]  config 配置
 * @return 错误码
 */
linuxhud_error_t linuxhud_create(linuxhud_t **out,
                                 const linuxhud_config_t *config);

/**
 * 显示 HUD（首次上屏）
 * 必须在 create 之后调用一次，之后用 update 刷新内容
 */
linuxhud_error_t linuxhud_show(linuxhud_t *hud);

/**
 * 清除 HUD 为背景色
 */
void linuxhud_clear(linuxhud_t *hud);

/**
 * 获取 HUD 的渲染器（用于自定义绘制）
 * @return 渲染器指针，生命周期与 HUD 一致
 */
linuxhud_renderer_t *linuxhud_get_renderer(linuxhud_t *hud);

/**
 * 获取 HUD 的画布尺寸
 */
void linuxhud_get_canvas_size(const linuxhud_t *hud,
                              uint32_t *width, uint32_t *height);

/**
 * 将当前 framebuffer 内容提交到显示器
 * 调用此函数后，所有绘制操作才会在屏幕上可见
 */
linuxhud_error_t linuxhud_present(linuxhud_t *hud);

/**
 * 设置 HUD 位置
 */
linuxhud_error_t linuxhud_set_position(linuxhud_t *hud, int32_t x, int32_t y);

/**
 * 设置 HUD 尺寸（会重新分配 buffer，代价较大）
 */
linuxhud_error_t linuxhud_set_size(linuxhud_t *hud, uint32_t width, uint32_t height);

/**
 * 设置 HUD 透明度
 */
void linuxhud_set_alpha(linuxhud_t *hud, uint8_t alpha);

/**
 * 设置 HUD 背景色
 */
void linuxhud_set_bg_color(linuxhud_t *hud, linuxhud_color_t color);

/* ---- 便捷绘制函数 ---- */

/**
 * 在 HUD 上绘制文本（一步到位：清除 → 绘制 → 提交）
 */
linuxhud_error_t linuxhud_draw_text(linuxhud_t *hud, const char *text,
                                    const linuxhud_text_style_t *style);

/**
 * 在 HUD 上绘制 Widget
 */
linuxhud_error_t linuxhud_draw_widget(linuxhud_t *hud, linuxhud_widget_t *widget);

/**
 * 销毁 HUD 实例，释放所有资源
 * @param hud HUD 句柄（可以为 NULL）
 */
void linuxhud_destroy(linuxhud_t *hud);

/**
 * 获取 HUD 当前 DRM 信息
 */
void linuxhud_get_drm_info(const linuxhud_t *hud, linuxhud_drm_info_t *info);

#endif /* LINUXHUD_HUD_H */
