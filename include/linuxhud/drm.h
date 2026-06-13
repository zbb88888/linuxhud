#ifndef LINUXHUD_DRM_H
#define LINUXHUD_DRM_H

#include "types.h"

/* ========================================================================
 * DRM 设备管理
 *
 * 职责:
 *   - 自动探测并打开 DRM 设备
 *   - 发现 Connector / CRTC / Plane
 *   - 管理 Framebuffer 生命周期
 * ======================================================================== */

/* DRM 设备配置 */
typedef struct {
    const char *device_path;      /* NULL = 自动探测 */
    uint32_t    preferred_plane;  /* 0=overlay, 1=cursor, 2=primary */
} linuxhud_drm_config_t;

/* DRM 设备句柄 (不透明类型) */
typedef struct linuxhud_drm linuxhud_drm_t;

/* DRM 连接器信息 */
typedef struct {
    uint32_t connector_id;
    uint32_t crtc_id;
    uint32_t plane_id;
    uint32_t width;               /* 显示器物理分辨率宽 */
    uint32_t height;              /* 显示器物理分辨率高 */
    uint32_t refresh_rate;        /* Hz */
    const char *name;             /* 如 "HDMI-A-1" */
} linuxhud_drm_info_t;

/**
 * 创建并打开 DRM 设备
 * @param[out] out    返回设备句柄
 * @param[in]  config 配置（NULL 使用默认）
 * @return 错误码
 */
linuxhud_error_t linuxhud_drm_open(linuxhud_drm_t **out,
                                   const linuxhud_drm_config_t *config);

/**
 * 获取设备信息
 * @param drm  设备句柄
 * @param[out] info  返回连接器信息
 */
void linuxhud_drm_get_info(const linuxhud_drm_t *drm, linuxhud_drm_info_t *info);

/**
 * 关闭并释放 DRM 设备
 * @param drm 设备句柄（可以为 NULL）
 */
void linuxhud_drm_close(linuxhud_drm_t *drm);

/* ========================================================================
 * Framebuffer 管理
 * ======================================================================== */

/* Framebuffer 句柄 (不透明类型) */
typedef struct linuxhud_fb linuxhud_fb_t;

/**
 * 创建 Framebuffer
 * @param drm    DRM 设备句柄
 * @param[out] out 返回 FB 句柄
 * @param width  像素宽度
 * @param height 像素高度
 * @return 错误码
 */
linuxhud_error_t linuxhud_fb_create(linuxhud_drm_t *drm, linuxhud_fb_t **out,
                                    uint32_t width, uint32_t height);

/**
 * 获取 framebuffer 的像素数据指针（可直接写入）
 * @param fb framebuffer 句柄
 * @return 像素数据指针 (ARGB8888)
 */
void *linuxhud_fb_get_data(linuxhud_fb_t *fb);

/**
 * 获取 framebuffer 的 stride（每行字节数）
 */
uint32_t linuxhud_fb_get_stride(linuxhud_fb_t *fb);

/**
 * 获取 framebuffer 尺寸
 */
uint32_t linuxhud_fb_get_width(const linuxhud_fb_t *fb);
uint32_t linuxhud_fb_get_height(const linuxhud_fb_t *fb);

/**
 * 提交 framebuffer 到显示器（显示到屏幕）
 * @param drm DRM 设备句柄
 * @param fb  要显示的 framebuffer
 * @param x   屏幕 X 坐标
 * @param y   屏幕 Y 坐标
 * @return 错误码
 */
linuxhud_error_t linuxhud_fb_present(linuxhud_drm_t *drm, linuxhud_fb_t *fb,
                                     int32_t x, int32_t y);

/**
 * 释放 framebuffer
 * @param fb framebuffer 句柄（可以为 NULL）
 */
void linuxhud_fb_destroy(linuxhud_fb_t *fb);

#endif /* LINUXHUD_DRM_H */
