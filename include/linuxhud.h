#ifndef LINUXHUD_H
#define LINUXHUD_H

#include <stdint.h>
#include <stdbool.h>

// 版本信息
#define LINUXHUD_VERSION_MAJOR 0
#define LINUXHUD_VERSION_MINOR 1
#define LINUXHUD_VERSION_PATCH 0

// 错误码
typedef enum {
    LINUXHUD_OK = 0,
    LINUXHUD_ERROR_DEVICE,
    LINUXHUD_ERROR_RESOURCE,
    LINUXHUD_ERROR_PLANE,
    LINUXHUD_ERROR_BUFFER,
    LINUXHUD_ERROR_RENDER,
    LINUXHUD_ERROR_PERMISSION
} linuxhud_error_t;

// 平面类型
typedef enum {
    LINUXHUD_PLANE_OVERLAY = 0,  // 硬件叠加平面（优先）
    LINUXHUD_PLANE_CURSOR = 1,   // 光标平面
    LINUXHUD_PLANE_PRIMARY = 2   // 主平面（最后选择）
} linuxhud_plane_type_t;

// 显示配置
typedef struct {
    uint32_t width;
    uint32_t height;
    uint32_t x;                  // X 坐标
    uint32_t y;                  // Y 坐标
    uint32_t refresh_rate;
    uint32_t alpha;              // 透明度 0-255
    uint32_t z_order;            // Z轴顺序
    linuxhud_plane_type_t preferred_plane;
} linuxhud_config_t;

// HUD 状态
typedef enum {
    LINUXHUD_STATE_INIT = 0,
    LINUXHUD_STATE_READY,
    LINUXHUD_STATE_RUNNING,
    LINUXHUD_STATE_PAUSED,
    LINUXHUD_STATE_ERROR
} linuxhud_state_t;

// DRM 设备信息
typedef struct {
    int fd;
    uint32_t connector_id;
    uint32_t crtc_id;
    uint32_t plane_id;
    uint32_t width;
    uint32_t height;
    uint32_t fb_id;
    void *mapped_buffer;
    uint32_t buffer_size;
    uint32_t stride;
} linuxhud_drm_t;

// 渲染上下文
typedef struct {
    void *cairo_surface;
    void *cairo_context;
    void *pango_layout;
    uint32_t text_color;
    uint32_t bg_color;
    uint32_t font_size;
    char *font_family;
} linuxhud_render_t;

// HUD 实例
typedef struct {
    linuxhud_state_t state;
    linuxhud_drm_t drm;
    linuxhud_render_t render;
    linuxhud_config_t config;
    bool running;
    uint32_t frame_count;
} linuxhud_t;

// 核心 API

/**
 * 初始化 HUD 实例
 * @param hud HUD 实例指针
 * @param config 配置参数
 * @return 错误码
 */
linuxhud_error_t linuxhud_init(linuxhud_t *hud, const linuxhud_config_t *config);

/**
 * 启动 HUD 渲染循环
 * @param hud HUD 实例指针
 * @return 错误码
 */
linuxhud_error_t linuxhud_start(linuxhud_t *hud);

/**
 * 停止 HUD
 * @param hud HUD 实例指针
 */
void linuxhud_stop(linuxhud_t *hud);

/**
 * 更新 HUD 显示内容
 * @param hud HUD 实例指针
 * @param text 要显示的文本
 * @return 错误码
 */
linuxhud_error_t linuxhud_update(linuxhud_t *hud, const char *text);

/**
 * 设置 HUD 位置
 * @param hud HUD 实例指针
 * @param x X 坐标
 * @param y Y 坐标
 * @return 错误码
 */
linuxhud_error_t linuxhud_set_position(linuxhud_t *hud, int32_t x, int32_t y);

/**
 * 设置 HUD 大小
 * @param hud HUD 实例指针
 * @param width 宽度
 * @param height 高度
 * @return 错误码
 */
linuxhud_error_t linuxhud_set_size(linuxhud_t *hud, uint32_t width, uint32_t height);

/**
 * 设置 HUD 透明度
 * @param hud HUD 实例指针
 * @param alpha 透明度 0-255
 * @return 错误码
 */
linuxhud_error_t linuxhud_set_alpha(linuxhud_t *hud, uint32_t alpha);

/**
 * 清理 HUD 资源
 * @param hud HUD 实例指针
 */
void linuxhud_cleanup(linuxhud_t *hud);

/**
 * 获取错误描述
 * @param error 错误码
 * @return 错误描述字符串
 */
const char *linuxhud_error_string(linuxhud_error_t error);

/**
 * 获取 HUD 状态
 * @param hud HUD 实例指针
 * @return 当前状态
 */
linuxhud_state_t linuxhud_get_state(const linuxhud_t *hud);

// DRM 设备管理函数
linuxhud_error_t linuxhud_drm_init(linuxhud_drm_t *drm);
void linuxhud_drm_cleanup(linuxhud_drm_t *drm);

// 缓冲区管理函数
linuxhud_error_t linuxhud_buffer_create(linuxhud_drm_t *drm, uint32_t width, uint32_t height);
linuxhud_error_t linuxhud_buffer_resize(linuxhud_drm_t *drm, uint32_t width, uint32_t height);
void linuxhud_buffer_clear(linuxhud_drm_t *drm, uint32_t color);
void linuxhud_buffer_fill_rect(linuxhud_drm_t *drm, int x, int y, int width, int height, uint32_t color);
void linuxhud_buffer_set_pixel(linuxhud_drm_t *drm, int x, int y, uint32_t color);
uint32_t linuxhud_buffer_get_pixel(linuxhud_drm_t *drm, int x, int y);
void linuxhud_buffer_destroy(linuxhud_drm_t *drm);

// 渲染器函数
linuxhud_error_t linuxhud_renderer_init(linuxhud_render_t *render, void *buffer, uint32_t width, uint32_t height, uint32_t stride);
void linuxhud_renderer_clear(linuxhud_render_t *render, uint32_t width, uint32_t height);
void linuxhud_renderer_draw_text(linuxhud_render_t *render, const char *text, int x, int y, uint32_t width, uint32_t height);
void linuxhud_renderer_draw_rect(linuxhud_render_t *render, int x, int y, int width, int height, uint32_t color, bool fill);
void linuxhud_renderer_draw_rounded_rect(linuxhud_render_t *render, int x, int y, int width, int height, int radius, uint32_t color, bool fill);
void linuxhud_renderer_set_text_color(linuxhud_render_t *render, uint32_t color);
void linuxhud_renderer_set_bg_color(linuxhud_render_t *render, uint32_t color);
void linuxhud_renderer_set_font(linuxhud_render_t *render, const char *family, uint32_t size);
void linuxhud_renderer_cleanup(linuxhud_render_t *render);

#endif // LINUXHUD_H
