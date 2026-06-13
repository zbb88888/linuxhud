#include "linuxhud/hud.h"
#include "linuxhud/types.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ========================================================================
 * HUD 内部结构
 * ======================================================================== */

struct linuxhud {
    /* 配置 */
    linuxhud_config_t  config;

    /* DRM */
    linuxhud_drm_t    *drm;
    linuxhud_fb_t     *fb;

    /* 渲染 */
    linuxhud_renderer_t *renderer;

    /* 状态 */
    bool shown;  /* 是否已上屏 */
};

/* ========================================================================
 * 公共 API
 * ======================================================================== */

void linuxhud_config_defaults(linuxhud_config_t *config) {
    if (!config) return;

    memset(config, 0, sizeof(*config));
    config->x         = 10;
    config->y         = 10;
    config->width     = 400;
    config->height    = 120;
    config->alpha     = 200;
    config->bg_color  = LINUXHUD_COLOR_BLACK;
    config->drm_config = NULL;
}

linuxhud_error_t linuxhud_create(linuxhud_t **out,
                                 const linuxhud_config_t *config) {
    if (!out || !config) return LINUXHUD_ERROR_INVAL;
    if (config->width == 0 || config->height == 0) return LINUXHUD_ERROR_INVAL;

    linuxhud_t *hud = calloc(1, sizeof(*hud));
    if (!hud) return LINUXHUD_ERROR_NOMEM;

    hud->config = *config;

    /* 打开 DRM 设备 */
    linuxhud_error_t err = linuxhud_drm_open(&hud->drm, config->drm_config);
    if (err != LINUXHUD_OK) {
        LINUXHUD_ERROR("Failed to open DRM device: %s", linuxhud_error_string(err));
        free(hud);
        return err;
    }

    /* 创建 framebuffer */
    err = linuxhud_fb_create(hud->drm, &hud->fb, config->width, config->height);
    if (err != LINUXHUD_OK) {
        LINUXHUD_ERROR("Failed to create framebuffer: %s", linuxhud_error_string(err));
        linuxhud_drm_close(hud->drm);
        free(hud);
        return err;
    }

    /* 创建渲染器 */
    err = linuxhud_renderer_create(&hud->renderer,
                                   linuxhud_fb_get_data(hud->fb),
                                   config->width, config->height,
                                   linuxhud_fb_get_stride(hud->fb));
    if (err != LINUXHUD_OK) {
        LINUXHUD_ERROR("Failed to create renderer: %s", linuxhud_error_string(err));
        linuxhud_fb_destroy(hud->fb);
        linuxhud_drm_close(hud->drm);
        free(hud);
        return err;
    }

    LINUXHUD_DEBUG("HUD created: %ux%u at (%d,%d)", config->width, config->height,
                  config->x, config->y);

    *out = hud;
    return LINUXHUD_OK;
}

linuxhud_error_t linuxhud_show(linuxhud_t *hud) {
    if (!hud) return LINUXHUD_ERROR_INVAL;

    /* 清除为背景色 */
    linuxhud_renderer_clear(hud->renderer, hud->config.bg_color);

    /* 提交到屏幕 */
    linuxhud_error_t err = linuxhud_fb_present(hud->drm, hud->fb,
                                               hud->config.x, hud->config.y);
    if (err == LINUXHUD_OK) {
        hud->shown = true;
    }
    return err;
}

void linuxhud_clear(linuxhud_t *hud) {
    if (!hud || !hud->renderer) return;
    linuxhud_renderer_clear(hud->renderer, hud->config.bg_color);
}

linuxhud_renderer_t *linuxhud_get_renderer(linuxhud_t *hud) {
    return hud ? hud->renderer : NULL;
}

void linuxhud_get_canvas_size(const linuxhud_t *hud,
                              uint32_t *width, uint32_t *height) {
    if (!hud) {
        if (width)  *width = 0;
        if (height) *height = 0;
        return;
    }
    if (width)  *width  = hud->config.width;
    if (height) *height = hud->config.height;
}

linuxhud_error_t linuxhud_present(linuxhud_t *hud) {
    if (!hud) return LINUXHUD_ERROR_INVAL;
    return linuxhud_fb_present(hud->drm, hud->fb,
                               hud->config.x, hud->config.y);
}

linuxhud_error_t linuxhud_set_position(linuxhud_t *hud, int32_t x, int32_t y) {
    if (!hud) return LINUXHUD_ERROR_INVAL;

    hud->config.x = x;
    hud->config.y = y;

    if (hud->shown) {
        return linuxhud_fb_present(hud->drm, hud->fb, x, y);
    }
    return LINUXHUD_OK;
}

linuxhud_error_t linuxhud_set_size(linuxhud_t *hud, uint32_t width, uint32_t height) {
    if (!hud || width == 0 || height == 0) return LINUXHUD_ERROR_INVAL;

    /* 尺寸没变则直接返回 */
    if (hud->config.width == width && hud->config.height == height) {
        return LINUXHUD_OK;
    }

    /* 销毁旧渲染器和 FB */
    linuxhud_renderer_destroy(hud->renderer);
    linuxhud_fb_destroy(hud->fb);
    hud->renderer = NULL;
    hud->fb = NULL;

    /* 创建新 FB */
    linuxhud_error_t err = linuxhud_fb_create(hud->drm, &hud->fb, width, height);
    if (err != LINUXHUD_OK) {
        return err;
    }

    /* 创建新渲染器 */
    err = linuxhud_renderer_create(&hud->renderer,
                                   linuxhud_fb_get_data(hud->fb),
                                   width, height,
                                   linuxhud_fb_get_stride(hud->fb));
    if (err != LINUXHUD_OK) {
        linuxhud_fb_destroy(hud->fb);
        hud->fb = NULL;
        return err;
    }

    hud->config.width  = width;
    hud->config.height = height;

    LINUXHUD_DEBUG("HUD resized to %ux%u", width, height);
    return LINUXHUD_OK;
}

void linuxhud_set_alpha(linuxhud_t *hud, uint8_t alpha) {
    if (hud) hud->config.alpha = alpha;
}

void linuxhud_set_bg_color(linuxhud_t *hud, linuxhud_color_t color) {
    if (hud) hud->config.bg_color = color;
}

/* ---- 便捷绘制函数 ---- */

linuxhud_error_t linuxhud_draw_text(linuxhud_t *hud, const char *text,
                                    const linuxhud_text_style_t *style) {
    if (!hud || !text || !style) return LINUXHUD_ERROR_INVAL;

    /* 清除背景 */
    linuxhud_renderer_clear(hud->renderer, hud->config.bg_color);

    /* 绘制文本 */
    linuxhud_rect_t rect = {
        .x = 10,
        .y = 10,
        .w = hud->config.width - 20,
        .h = hud->config.height - 20,
    };
    linuxhud_renderer_draw_text(hud->renderer, text, &rect, style);

    /* 提交 */
    return linuxhud_present(hud);
}

linuxhud_error_t linuxhud_draw_widget(linuxhud_t *hud, linuxhud_widget_t *widget) {
    if (!hud || !widget) return LINUXHUD_ERROR_INVAL;

    linuxhud_widget_draw(widget, hud->renderer);
    return linuxhud_present(hud);
}

void linuxhud_destroy(linuxhud_t *hud) {
    if (!hud) return;

    LINUXHUD_DEBUG("Destroying HUD");

    linuxhud_renderer_destroy(hud->renderer);
    linuxhud_fb_destroy(hud->fb);
    linuxhud_drm_close(hud->drm);
    free(hud);
}

void linuxhud_get_drm_info(const linuxhud_t *hud, linuxhud_drm_info_t *info) {
    if (!hud || !info) return;
    linuxhud_drm_get_info(hud->drm, info);
}
