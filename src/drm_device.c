#include "linuxhud.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <xf86drm.h>
#include <xf86drmMode.h>
#include <drm_fourcc.h>

#define DRM_DEVICE_PATH "/dev/dri/card1"

// 未使用参数宏
#define UNUSED(x) ((void)(x))

// 查找已连接的连接器
static int find_connected_connector(int fd, drmModeRes *resources, 
                                   uint32_t *connector_id, uint32_t *crtc_id) {
    for (int i = 0; i < resources->count_connectors; i++) {
        drmModeConnector *connector = drmModeGetConnector(fd, resources->connectors[i]);
        if (!connector) continue;
        
        if (connector->connection == DRM_MODE_CONNECTED && connector->count_modes > 0) {
            *connector_id = connector->connector_id;
            
            // 查找对应的编码器
            drmModeEncoder *encoder = NULL;
            if (connector->encoder_id) {
                encoder = drmModeGetEncoder(fd, connector->encoder_id);
            }
            
            if (encoder && encoder->crtc_id) {
                *crtc_id = encoder->crtc_id;
                drmModeFreeEncoder(encoder);
            } else {
                // 查找可用的 CRTC
                uint32_t possible_crtcs = 0;
                if (encoder) {
                    possible_crtcs = encoder->possible_crtcs;
                    drmModeFreeEncoder(encoder);
                } else {
                    // 如果没有编码器，尝试所有 CRTC
                    possible_crtcs = (1 << resources->count_crtcs) - 1;
                }
                
                // 查找第一个可用的 CRTC
                for (int j = 0; j < resources->count_crtcs; j++) {
                    if (possible_crtcs & (1 << j)) {
                        *crtc_id = resources->crtcs[j];
                        break;
                    }
                }
                
                // 如果仍然没有找到，使用第一个 CRTC
                if (*crtc_id == 0 && resources->count_crtcs > 0) {
                    *crtc_id = resources->crtcs[0];
                }
            }
            
            drmModeFreeConnector(connector);
            return 0;
        }
        drmModeFreeConnector(connector);
    }
    return -1;
}

// 查找 Overlay Plane
static int find_overlay_plane(int fd, uint32_t crtc_id, uint32_t *plane_id) {
    UNUSED(crtc_id);  // 直接使用第一个可用的平面
    drmModePlaneRes *plane_resources = drmModeGetPlaneResources(fd);
    if (!plane_resources) return -1;
    
    // 首先尝试查找 Overlay Plane
    for (uint32_t i = 0; i < plane_resources->count_planes; i++) {
        drmModePlane *plane = drmModeGetPlane(fd, plane_resources->planes[i]);
        if (!plane) continue;
        
        // 检查平面类型
        drmModeObjectProperties *props = drmModeObjectGetProperties(fd, 
            plane->plane_id, DRM_MODE_OBJECT_PLANE);
        if (props) {
            for (uint32_t j = 0; j < props->count_props; j++) {
                drmModePropertyPtr prop = drmModeGetProperty(fd, props->props[j]);
                if (prop) {
                    if (strcmp(prop->name, "type") == 0) {
                        if (props->prop_values[j] == DRM_PLANE_TYPE_OVERLAY) {
                            // 直接使用第一个可用的 Overlay Plane
                            *plane_id = plane->plane_id;
                            drmModeFreeProperty(prop);
                            drmModeFreeObjectProperties(props);
                            drmModeFreePlane(plane);
                            drmModeFreePlaneResources(plane_resources);
                            return 0;
                        }
                    }
                    drmModeFreeProperty(prop);
                }
            }
            drmModeFreeObjectProperties(props);
        }
        drmModeFreePlane(plane);
    }
    
    // 如果没有 Overlay，尝试 Cursor Plane
    for (uint32_t i = 0; i < plane_resources->count_planes; i++) {
        drmModePlane *plane = drmModeGetPlane(fd, plane_resources->planes[i]);
        if (!plane) continue;
        
        drmModeObjectProperties *props = drmModeObjectGetProperties(fd, 
            plane->plane_id, DRM_MODE_OBJECT_PLANE);
        if (props) {
            for (uint32_t j = 0; j < props->count_props; j++) {
                drmModePropertyPtr prop = drmModeGetProperty(fd, props->props[j]);
                if (prop) {
                    if (strcmp(prop->name, "type") == 0) {
                        if (props->prop_values[j] == DRM_PLANE_TYPE_CURSOR) {
                            // 直接使用第一个可用的 Cursor Plane
                            *plane_id = plane->plane_id;
                            drmModeFreeProperty(prop);
                            drmModeFreeObjectProperties(props);
                            drmModeFreePlane(plane);
                            drmModeFreePlaneResources(plane_resources);
                            return 0;
                        }
                    }
                    drmModeFreeProperty(prop);
                }
            }
            drmModeFreeObjectProperties(props);
        }
        drmModeFreePlane(plane);
    }
    
    drmModeFreePlaneResources(plane_resources);
    return -1;
}

// 创建 Dumb Buffer
static int create_dumb_buffer(int fd, uint32_t width, uint32_t height, 
                             uint32_t *fb_id, void **mapped, uint32_t *size, uint32_t *stride) {
    struct drm_mode_create_dumb creq = {0};
    struct drm_mode_map_dumb mreq = {0};
    
    creq.width = width;
    creq.height = height;
    creq.bpp = 32;  // ARGB8888
    
    if (drmIoctl(fd, DRM_IOCTL_MODE_CREATE_DUMB, &creq)) {
        return -1;
    }
    
    // 创建 Framebuffer
    uint32_t handles[4] = {creq.handle};
    uint32_t pitches[4] = {creq.pitch};
    uint32_t offsets[4] = {0};
    
    if (drmModeAddFB2(fd, width, height, DRM_FORMAT_ARGB8888,
                      handles, pitches, offsets, fb_id, 0)) {
        return -1;
    }
    
    // 映射到用户空间
    mreq.handle = creq.handle;
    if (drmIoctl(fd, DRM_IOCTL_MODE_MAP_DUMB, &mreq)) {
        return -1;
    }
    
    *mapped = mmap(0, creq.size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, mreq.offset);
    if (*mapped == MAP_FAILED) {
        return -1;
    }
    
    *size = creq.size;
    *stride = creq.pitch;
    
    return 0;
}

linuxhud_error_t linuxhud_drm_init(linuxhud_drm_t *drm) {
    // 打开 DRM 设备
    drm->fd = open(DRM_DEVICE_PATH, O_RDWR);
    if (drm->fd < 0) {
        perror("Failed to open DRM device");
        return LINUXHUD_ERROR_DEVICE;
    }
    
    // 启用通用平面能力
    if (drmSetClientCap(drm->fd, DRM_CLIENT_CAP_UNIVERSAL_PLANES, 1)) {
        close(drm->fd);
        return LINUXHUD_ERROR_RESOURCE;
    }
    
    // 获取资源
    drmModeRes *resources = drmModeGetResources(drm->fd);
    if (!resources) {
        close(drm->fd);
        return LINUXHUD_ERROR_RESOURCE;
    }
    
    // 查找连接器和 CRTC
    if (find_connected_connector(drm->fd, resources, &drm->connector_id, &drm->crtc_id)) {
        drmModeFreeResources(resources);
        close(drm->fd);
        return LINUXHUD_ERROR_RESOURCE;
    }
    
    // 获取显示模式
    drmModeConnector *connector = drmModeGetConnector(drm->fd, drm->connector_id);
    if (connector && connector->count_modes > 0) {
        drm->width = connector->modes[0].hdisplay;
        drm->height = connector->modes[0].vdisplay;
        drmModeFreeConnector(connector);
    } else {
        drmModeFreeResources(resources);
        close(drm->fd);
        return LINUXHUD_ERROR_RESOURCE;
    }
    
    // 查找 Overlay Plane
    if (find_overlay_plane(drm->fd, drm->crtc_id, &drm->plane_id)) {
        drmModeFreeResources(resources);
        close(drm->fd);
        return LINUXHUD_ERROR_PLANE;
    }
    
    drmModeFreeResources(resources);
    
    printf("DRM initialized: Connector=%u, CRTC=%u, Plane=%u, Resolution=%ux%u\n",
           drm->connector_id, drm->crtc_id, drm->plane_id, drm->width, drm->height);
    
    return LINUXHUD_OK;
}

linuxhud_error_t linuxhud_drm_create_buffer(linuxhud_drm_t *drm, uint32_t width, uint32_t height) {
    if (create_dumb_buffer(drm->fd, width, height, &drm->fb_id, 
                          &drm->mapped_buffer, &drm->buffer_size, &drm->stride)) {
        return LINUXHUD_ERROR_BUFFER;
    }
    
    printf("Buffer created: FB=%u, Size=%u, Stride=%u\n", 
           drm->fb_id, drm->buffer_size, drm->stride);
    
    return LINUXHUD_OK;
}

void linuxhud_drm_cleanup(linuxhud_drm_t *drm) {
    if (drm->mapped_buffer && drm->mapped_buffer != MAP_FAILED) {
        munmap(drm->mapped_buffer, drm->buffer_size);
    }
    
    if (drm->fb_id) {
        drmModeRmFB(drm->fd, drm->fb_id);
    }
    
    if (drm->fd >= 0) {
        close(drm->fd);
    }
}
