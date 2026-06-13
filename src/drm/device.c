#include "linuxhud/drm.h"
#include "linuxhud/types.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/mman.h>
#include <errno.h>
#include <xf86drm.h>
#include <xf86drmMode.h>
#include <drm_fourcc.h>

/* ========================================================================
 * 内部数据结构
 * ======================================================================== */

struct linuxhud_drm {
    int              fd;
    uint32_t         connector_id;
    uint32_t         crtc_id;
    uint32_t         plane_id;
    uint32_t         screen_width;
    uint32_t         screen_height;
    uint32_t         refresh_rate;
    char             connector_name[64];
    bool             owns_fd;       /* 是否由我们打开（需要负责关闭） */
};

struct linuxhud_fb {
    linuxhud_drm_t  *drm;
    uint32_t         fb_id;
    uint32_t         handle;       /* dumb buffer handle */
    void            *data;         /* mmap 指针 */
    uint32_t         size;
    uint32_t         stride;
    uint32_t         width;
    uint32_t         height;
};

/* ========================================================================
 * DRM 设备自动探测
 * ======================================================================== */

/* 候选设备路径列表 */
static const char *candidate_paths[] = {
    "/dev/dri/card0",
    "/dev/dri/card1",
    "/dev/dri/card2",
    "/dev/dri/card3",
    NULL
};

/**
 * 尝试打开并验证一个 DRM 设备
 * @return fd，失败返回 -1
 */
static int try_open_device(const char *path) {
    int fd = open(path, O_RDWR);
    if (fd < 0) {
        return -1;
    }

    /* 验证是有效的 DRM 设备 */
    drmModeRes *res = drmModeGetResources(fd);
    if (!res) {
        close(fd);
        return -1;
    }

    drmModeFreeResources(res);
    return fd;
}

/**
 * 查找第一个已连接的 connector 及其 CRTC
 */
static int find_connector_and_crtc(int fd, drmModeRes *resources,
                                   uint32_t *out_conn, uint32_t *out_crtc,
                                   uint32_t *out_w, uint32_t *out_h,
                                   uint32_t *out_refresh, char *out_name,
                                   size_t name_len) {
    for (int i = 0; i < resources->count_connectors; i++) {
        drmModeConnector *conn = drmModeGetConnector(fd, resources->connectors[i]);
        if (!conn) continue;

        if (conn->connection != DRM_MODE_CONNECTED || conn->count_modes == 0) {
            drmModeFreeConnector(conn);
            continue;
        }

        *out_conn = conn->connector_id;

        /* 保存首选模式信息 */
        *out_w = conn->modes[0].hdisplay;
        *out_h = conn->modes[0].vdisplay;
        *out_refresh = conn->modes[0].vrefresh;

        /* 生成连接器名称 */
        const char *type_str = "Unknown";
        switch (conn->connector_type) {
            case DRM_MODE_CONNECTOR_HDMIA: type_str = "HDMI-A"; break;
            case DRM_MODE_CONNECTOR_HDMIB: type_str = "HDMI-B"; break;
            case DRM_MODE_CONNECTOR_DisplayPort: type_str = "DP"; break;
            case DRM_MODE_CONNECTOR_eDP:   type_str = "eDP"; break;
            case DRM_MODE_CONNECTOR_VGA:   type_str = "VGA"; break;
            case DRM_MODE_CONNECTOR_DVII:
            case DRM_MODE_CONNECTOR_DVID:
            case DRM_MODE_CONNECTOR_DVIA:  type_str = "DVI"; break;
            default: break;
        }
        snprintf(out_name, name_len, "%s-%u", type_str, conn->connector_type_id);

        /* 查找 CRTC */
        *out_crtc = 0;

        /* 优先使用当前 encoder 的 CRTC */
        if (conn->encoder_id) {
            drmModeEncoder *enc = drmModeGetEncoder(fd, conn->encoder_id);
            if (enc && enc->crtc_id) {
                *out_crtc = enc->crtc_id;
                drmModeFreeEncoder(enc);
                drmModeFreeConnector(conn);
                LINUXHUD_DEBUG("Using encoder's CRTC %u for connector %s",
                              *out_crtc, out_name);
                return 0;
            }
            if (enc) drmModeFreeEncoder(enc);
        }

        /* 否则从 possible_crtcs 中选择第一个 */
        drmModeEncoder *enc = NULL;
        uint32_t possible_crtcs = 0;
        if (conn->encoder_id) {
            enc = drmModeGetEncoder(fd, conn->encoder_id);
            if (enc) {
                possible_crtcs = enc->possible_crtcs;
                drmModeFreeEncoder(enc);
            }
        }
        if (possible_crtcs == 0) {
            /* 尝试所有 CRTC */
            possible_crtcs = (1u << resources->count_crtcs) - 1;
        }

        for (int j = 0; j < resources->count_crtcs; j++) {
            if (possible_crtcs & (1u << j)) {
                *out_crtc = resources->crtcs[j];
                break;
            }
        }

        if (*out_crtc == 0 && resources->count_crtcs > 0) {
            *out_crtc = resources->crtcs[0];
        }

        drmModeFreeConnector(conn);
        LINUXHUD_DEBUG("Found connector %s: %ux%u@%uHz, CRTC %u",
                      out_name, *out_w, *out_h, *out_refresh, *out_crtc);
        return 0;
    }

    return -1;
}

/**
 * 查找合适的 overlay plane
 */
static int find_plane(int fd, uint32_t crtc_id, uint32_t preferred,
                      uint32_t *out_plane) {
    LINUXHUD_UNUSED(crtc_id);  /* TODO: 未来可用于检查 plane 的 possible_crtcs */
    drmModePlaneRes *planes = drmModeGetPlaneResources(fd);
    if (!planes) return -1;

    /* 按优先级查找: overlay → cursor → primary */
    static const uint32_t type_order[] = {
        DRM_PLANE_TYPE_OVERLAY,
        DRM_PLANE_TYPE_CURSOR,
        DRM_PLANE_TYPE_PRIMARY,
    };

    /* 如果用户指定了首选类型，把它排到最前面 */
    uint32_t search_order[3];
    if (preferred < 3) {
        search_order[0] = type_order[preferred];
        search_order[1] = type_order[(preferred + 1) % 3];
        search_order[2] = type_order[(preferred + 2) % 3];
    } else {
        search_order[0] = type_order[0];
        search_order[1] = type_order[1];
        search_order[2] = type_order[2];
    }

    for (int t = 0; t < 3; t++) {
        for (uint32_t i = 0; i < planes->count_planes; i++) {
            drmModePlane *plane = drmModeGetPlane(fd, planes->planes[i]);
            if (!plane) continue;

            drmModeObjectProperties *props = drmModeObjectGetProperties(
                fd, plane->plane_id, DRM_MODE_OBJECT_PLANE);

            if (props) {
                for (uint32_t j = 0; j < props->count_props; j++) {
                    drmModePropertyPtr prop = drmModeGetProperty(fd, props->props[j]);
                    if (!prop) continue;

                    if (strcmp(prop->name, "type") == 0 &&
                        props->prop_values[j] == search_order[t]) {
                        *out_plane = plane->plane_id;
                        drmModeFreeProperty(prop);
                        drmModeFreeObjectProperties(props);
                        drmModeFreePlane(plane);
                        drmModeFreePlaneResources(planes);

                        const char *type_name = (search_order[t] == DRM_PLANE_TYPE_OVERLAY) ?
                            "OVERLAY" : (search_order[t] == DRM_PLANE_TYPE_CURSOR) ?
                            "CURSOR" : "PRIMARY";
                        LINUXHUD_DEBUG("Found %s plane %u", type_name, *out_plane);
                        return 0;
                    }
                    drmModeFreeProperty(prop);
                }
                drmModeFreeObjectProperties(props);
            }
            drmModeFreePlane(plane);
        }
    }

    drmModeFreePlaneResources(planes);
    return -1;
}

/* ========================================================================
 * 公共 API: DRM 设备
 * ======================================================================== */

linuxhud_error_t linuxhud_drm_open(linuxhud_drm_t **out,
                                   const linuxhud_drm_config_t *config) {
    if (!out) return LINUXHUD_ERROR_INVAL;

    linuxhud_drm_t *drm = calloc(1, sizeof(*drm));
    if (!drm) return LINUXHUD_ERROR_NOMEM;

    drm->fd = -1;
    drm->owns_fd = true;

    /* 打开设备 */
    if (config && config->device_path) {
        drm->fd = try_open_device(config->device_path);
        if (drm->fd < 0) {
            LINUXHUD_ERROR("Cannot open DRM device: %s", config->device_path);
            free(drm);
            return LINUXHUD_ERROR_DEVICE;
        }
    } else {
        /* 自动探测 */
        for (int i = 0; candidate_paths[i]; i++) {
            drm->fd = try_open_device(candidate_paths[i]);
            if (drm->fd >= 0) {
                LINUXHUD_INFO("Auto-detected DRM device: %s", candidate_paths[i]);
                break;
            }
        }
        if (drm->fd < 0) {
            LINUXHUD_ERROR("No DRM device found");
            free(drm);
            return LINUXHUD_ERROR_NOT_FOUND;
        }
    }

    /* 启用 universal planes */
    if (drmSetClientCap(drm->fd, DRM_CLIENT_CAP_UNIVERSAL_PLANES, 1)) {
        LINUXHUD_WARN("Failed to set UNIVERSAL_PLANES capability");
    }

    /* 获取资源 */
    drmModeRes *resources = drmModeGetResources(drm->fd);
    if (!resources) {
        LINUXHUD_ERROR("drmModeGetResources failed: %s", strerror(errno));
        close(drm->fd);
        free(drm);
        return LINUXHUD_ERROR_RESOURCE;
    }

    /* 查找连接器和 CRTC */
    if (find_connector_and_crtc(drm->fd, resources,
                                &drm->connector_id, &drm->crtc_id,
                                &drm->screen_width, &drm->screen_height,
                                &drm->refresh_rate,
                                drm->connector_name, sizeof(drm->connector_name))) {
        LINUXHUD_ERROR("No connected display found");
        drmModeFreeResources(resources);
        close(drm->fd);
        free(drm);
        return LINUXHUD_ERROR_NOT_FOUND;
    }

    /* 查找 plane */
    uint32_t preferred = (config) ? config->preferred_plane : 0;
    if (find_plane(drm->fd, drm->crtc_id, preferred, &drm->plane_id)) {
        LINUXHUD_ERROR("No suitable plane found");
        drmModeFreeResources(resources);
        close(drm->fd);
        free(drm);
        return LINUXHUD_ERROR_PLANE;
    }

    drmModeFreeResources(resources);

    LINUXHUD_INFO("DRM ready: %s %ux%u@%uHz, plane %u",
                  drm->connector_name, drm->screen_width, drm->screen_height,
                  drm->refresh_rate, drm->plane_id);

    *out = drm;
    return LINUXHUD_OK;
}

void linuxhud_drm_get_info(const linuxhud_drm_t *drm, linuxhud_drm_info_t *info) {
    if (!drm || !info) return;

    info->connector_id = drm->connector_id;
    info->crtc_id      = drm->crtc_id;
    info->plane_id     = drm->plane_id;
    info->width        = drm->screen_width;
    info->height       = drm->screen_height;
    info->refresh_rate = drm->refresh_rate;
    info->name         = drm->connector_name;
}

void linuxhud_drm_close(linuxhud_drm_t *drm) {
    if (!drm) return;
    if (drm->fd >= 0 && drm->owns_fd) {
        close(drm->fd);
    }
    free(drm);
}

/* ========================================================================
 * 公共 API: Framebuffer
 * ======================================================================== */

linuxhud_error_t linuxhud_fb_create(linuxhud_drm_t *drm, linuxhud_fb_t **out,
                                    uint32_t width, uint32_t height) {
    if (!drm || !out || width == 0 || height == 0) {
        return LINUXHUD_ERROR_INVAL;
    }

    linuxhud_fb_t *fb = calloc(1, sizeof(*fb));
    if (!fb) return LINUXHUD_ERROR_NOMEM;

    fb->drm    = drm;
    fb->width  = width;
    fb->height = height;

    /* 创建 dumb buffer */
    struct drm_mode_create_dumb creq = {
        .width  = width,
        .height = height,
        .bpp    = 32,
    };

    if (drmIoctl(drm->fd, DRM_IOCTL_MODE_CREATE_DUMB, &creq)) {
        LINUXHUD_ERROR("CREATE_DUMB failed: %s", strerror(errno));
        free(fb);
        return LINUXHUD_ERROR_BUFFER;
    }

    fb->handle = creq.handle;
    fb->stride = creq.pitch;
    fb->size   = creq.size;

    /* 创建 framebuffer */
    uint32_t handles[4] = {creq.handle};
    uint32_t pitches[4] = {creq.pitch};
    uint32_t offsets[4] = {0};

    if (drmModeAddFB2(drm->fd, width, height, DRM_FORMAT_ARGB8888,
                      handles, pitches, offsets, &fb->fb_id, 0)) {
        LINUXHUD_ERROR("drmModeAddFB2 failed: %s", strerror(errno));
        /* 清理 dumb buffer */
        struct drm_mode_destroy_dumb dreq = {.handle = fb->handle};
        drmIoctl(drm->fd, DRM_IOCTL_MODE_DESTROY_DUMB, &dreq);
        free(fb);
        return LINUXHUD_ERROR_BUFFER;
    }

    /* 映射到用户空间 */
    struct drm_mode_map_dumb mreq = {.handle = fb->handle};
    if (drmIoctl(drm->fd, DRM_IOCTL_MODE_MAP_DUMB, &mreq)) {
        LINUXHUD_ERROR("MAP_DUMB failed: %s", strerror(errno));
        drmModeRmFB(drm->fd, fb->fb_id);
        struct drm_mode_destroy_dumb dreq = {.handle = fb->handle};
        drmIoctl(drm->fd, DRM_IOCTL_MODE_DESTROY_DUMB, &dreq);
        free(fb);
        return LINUXHUD_ERROR_BUFFER;
    }

    fb->data = mmap(NULL, fb->size, PROT_READ | PROT_WRITE, MAP_SHARED,
                    drm->fd, mreq.offset);
    if (fb->data == MAP_FAILED) {
        LINUXHUD_ERROR("mmap failed: %s", strerror(errno));
        drmModeRmFB(drm->fd, fb->fb_id);
        struct drm_mode_destroy_dumb dreq = {.handle = fb->handle};
        drmIoctl(drm->fd, DRM_IOCTL_MODE_DESTROY_DUMB, &dreq);
        free(fb);
        return LINUXHUD_ERROR_BUFFER;
    }

    /* 清零 */
    memset(fb->data, 0, fb->size);

    LINUXHUD_DEBUG("FB created: %ux%u, stride=%u, size=%u", width, height,
                  fb->stride, fb->size);

    *out = fb;
    return LINUXHUD_OK;
}

void *linuxhud_fb_get_data(linuxhud_fb_t *fb) {
    return fb ? fb->data : NULL;
}

uint32_t linuxhud_fb_get_stride(linuxhud_fb_t *fb) {
    return fb ? fb->stride : 0;
}

uint32_t linuxhud_fb_get_width(const linuxhud_fb_t *fb) {
    return fb ? fb->width : 0;
}

uint32_t linuxhud_fb_get_height(const linuxhud_fb_t *fb) {
    return fb ? fb->height : 0;
}

linuxhud_error_t linuxhud_fb_present(linuxhud_drm_t *drm, linuxhud_fb_t *fb,
                                     int32_t x, int32_t y) {
    if (!drm || !fb) return LINUXHUD_ERROR_INVAL;

    int ret = drmModeSetPlane(drm->fd, drm->plane_id, drm->crtc_id,
                              fb->fb_id, 0,
                              x, y, fb->width, fb->height,
                              0, 0, fb->width << 16, fb->height << 16);
    if (ret) {
        LINUXHUD_ERROR("drmModeSetPlane failed: %s (errno=%d)",
                      strerror(errno), errno);
        return LINUXHUD_ERROR_RENDER;
    }

    return LINUXHUD_OK;
}

void linuxhud_fb_destroy(linuxhud_fb_t *fb) {
    if (!fb) return;

    if (fb->data && fb->data != MAP_FAILED) {
        munmap(fb->data, fb->size);
    }
    if (fb->fb_id && fb->drm) {
        drmModeRmFB(fb->drm->fd, fb->fb_id);
    }
    if (fb->handle && fb->drm) {
        struct drm_mode_destroy_dumb dreq = {.handle = fb->handle};
        drmIoctl(fb->drm->fd, DRM_IOCTL_MODE_DESTROY_DUMB, &dreq);
    }
    free(fb);
}
