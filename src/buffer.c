#include "linuxhud.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <xf86drm.h>
#include <xf86drmMode.h>
#include <drm_fourcc.h>

linuxhud_error_t linuxhud_buffer_create(linuxhud_drm_t *drm, uint32_t width, uint32_t height) {
    struct drm_mode_create_dumb creq = {0};
    struct drm_mode_map_dumb mreq = {0};
    
    // 设置缓冲区参数
    creq.width = width;
    creq.height = height;
    creq.bpp = 32;  // ARGB8888
    
    // 创建 Dumb Buffer
    if (drmIoctl(drm->fd, DRM_IOCTL_MODE_CREATE_DUMB, &creq)) {
        perror("Failed to create dumb buffer");
        return LINUXHUD_ERROR_BUFFER;
    }
    
    // 创建 Framebuffer
    uint32_t handles[4] = {creq.handle};
    uint32_t pitches[4] = {creq.pitch};
    uint32_t offsets[4] = {0};
    
    if (drmModeAddFB2(drm->fd, width, height, DRM_FORMAT_ARGB8888,
                      handles, pitches, offsets, &drm->fb_id, 0)) {
        perror("Failed to create framebuffer");
        return LINUXHUD_ERROR_BUFFER;
    }
    
    // 映射到用户空间
    mreq.handle = creq.handle;
    if (drmIoctl(drm->fd, DRM_IOCTL_MODE_MAP_DUMB, &mreq)) {
        perror("Failed to map dumb buffer");
        return LINUXHUD_ERROR_BUFFER;
    }
    
    drm->mapped_buffer = mmap(0, creq.size, PROT_READ | PROT_WRITE, MAP_SHARED, 
                              drm->fd, mreq.offset);
    if (drm->mapped_buffer == MAP_FAILED) {
        perror("Failed to mmap buffer");
        return LINUXHUD_ERROR_BUFFER;
    }
    
    // 保存缓冲区信息
    drm->buffer_size = creq.size;
    drm->stride = creq.pitch;
    
    // 清空缓冲区
    memset(drm->mapped_buffer, 0, creq.size);
    
    printf("Buffer created: %ux%u, stride=%u, size=%u\n", 
           width, height, drm->stride, drm->buffer_size);
    
    return LINUXHUD_OK;
}

linuxhud_error_t linuxhud_buffer_resize(linuxhud_drm_t *drm, uint32_t width, uint32_t height) {
    // 清理旧缓冲区
    if (drm->mapped_buffer && drm->mapped_buffer != MAP_FAILED) {
        munmap(drm->mapped_buffer, drm->buffer_size);
    }
    
    if (drm->fb_id) {
        drmModeRmFB(drm->fd, drm->fb_id);
    }
    
    // 创建新缓冲区
    return linuxhud_buffer_create(drm, width, height);
}

void linuxhud_buffer_clear(linuxhud_drm_t *drm, uint32_t color) {
    if (!drm->mapped_buffer || drm->mapped_buffer == MAP_FAILED) {
        return;
    }
    
    // 逐像素填充
    uint32_t *pixels = (uint32_t *)drm->mapped_buffer;
    uint32_t pixel_count = drm->buffer_size / 4;
    
    for (uint32_t i = 0; i < pixel_count; i++) {
        pixels[i] = color;
    }
}

void linuxhud_buffer_fill_rect(linuxhud_drm_t *drm, 
                               int x, int y, int width, int height, uint32_t color) {
    if (!drm->mapped_buffer || drm->mapped_buffer == MAP_FAILED) {
        return;
    }
    
    uint32_t *pixels = (uint32_t *)drm->mapped_buffer;
    uint32_t stride_pixels = drm->stride / 4;
    
    // 边界检查
    if (x < 0) x = 0;
    if (y < 0) y = 0;
    if (x + width > (int)(drm->stride / 4)) width = drm->stride / 4 - x;
    if (y + height > (int)(drm->buffer_size / drm->stride)) height = drm->buffer_size / drm->stride - y;
    
    // 填充矩形区域
    for (int row = y; row < y + height; row++) {
        for (int col = x; col < x + width; col++) {
            pixels[row * stride_pixels + col] = color;
        }
    }
}

void linuxhud_buffer_set_pixel(linuxhud_drm_t *drm, int x, int y, uint32_t color) {
    if (!drm->mapped_buffer || drm->mapped_buffer == MAP_FAILED) {
        return;
    }
    
    uint32_t *pixels = (uint32_t *)drm->mapped_buffer;
    uint32_t stride_pixels = drm->stride / 4;
    
    if (x >= 0 && x < (int)stride_pixels && y >= 0 && y < (int)(drm->buffer_size / drm->stride)) {
        pixels[y * stride_pixels + x] = color;
    }
}

uint32_t linuxhud_buffer_get_pixel(linuxhud_drm_t *drm, int x, int y) {
    if (!drm->mapped_buffer || drm->mapped_buffer == MAP_FAILED) {
        return 0;
    }
    
    uint32_t *pixels = (uint32_t *)drm->mapped_buffer;
    uint32_t stride_pixels = drm->stride / 4;
    
    if (x >= 0 && x < (int)stride_pixels && y >= 0 && y < (int)(drm->buffer_size / drm->stride)) {
        return pixels[y * stride_pixels + x];
    }
    
    return 0;
}

void linuxhud_buffer_destroy(linuxhud_drm_t *drm) {
    if (drm->mapped_buffer && drm->mapped_buffer != MAP_FAILED) {
        munmap(drm->mapped_buffer, drm->buffer_size);
        drm->mapped_buffer = NULL;
    }
    
    if (drm->fb_id) {
        drmModeRmFB(drm->fd, drm->fb_id);
        drm->fb_id = 0;
    }
}
