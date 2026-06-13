#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/mman.h>
#include <xf86drm.h>
#include <xf86drmMode.h>
#include <drm_fourcc.h>
#include <errno.h>

int main() {
    int fd;
    drmModeRes *resources;
    drmModePlaneRes *plane_resources;
    uint32_t connector_id = 0;
    uint32_t crtc_id = 0;
    uint32_t plane_id = 0;
    uint32_t fb_id = 0;
    void *mapped = NULL;
    uint32_t width = 400;
    uint32_t height = 300;
    uint32_t stride = 0;
    uint32_t size = 0;
    
    // 打开 DRM 设备
    fd = open("/dev/dri/card1", O_RDWR);
    if (fd < 0) {
        perror("Failed to open DRM device");
        return 1;
    }
    
    printf("DRM device opened successfully\n");
    
    // 启用通用平面能力
    if (drmSetClientCap(fd, DRM_CLIENT_CAP_UNIVERSAL_PLANES, 1)) {
        printf("Failed to set universal planes capability\n");
        close(fd);
        return 1;
    }
    
    // 获取资源
    resources = drmModeGetResources(fd);
    if (!resources) {
        perror("Failed to get DRM resources");
        close(fd);
        return 1;
    }
    
    // 查找已连接的连接器
    for (int i = 0; i < resources->count_connectors; i++) {
        drmModeConnector *connector = drmModeGetConnector(fd, resources->connectors[i]);
        if (!connector) continue;
        
        if (connector->connection == DRM_MODE_CONNECTED && connector->count_modes > 0) {
            connector_id = connector->connector_id;
            printf("Found connected connector: %u\n", connector_id);
            
            // 查找对应的编码器
            drmModeEncoder *encoder = NULL;
            if (connector->encoder_id) {
                encoder = drmModeGetEncoder(fd, connector->encoder_id);
            }
            
            if (encoder && encoder->crtc_id) {
                crtc_id = encoder->crtc_id;
                printf("Using CRTC from encoder: %u\n", crtc_id);
                drmModeFreeEncoder(encoder);
            } else {
                // 查找可用的 CRTC
                uint32_t possible_crtcs = 0;
                if (encoder) {
                    possible_crtcs = encoder->possible_crtcs;
                    drmModeFreeEncoder(encoder);
                } else {
                    possible_crtcs = (1 << resources->count_crtcs) - 1;
                }
                
                for (int j = 0; j < resources->count_crtcs; j++) {
                    if (possible_crtcs & (1 << j)) {
                        crtc_id = resources->crtcs[j];
                        printf("Using CRTC: %u\n", crtc_id);
                        break;
                    }
                }
            }
            
            drmModeFreeConnector(connector);
            break;
        }
        drmModeFreeConnector(connector);
    }
    
    if (connector_id == 0 || crtc_id == 0) {
        printf("No connected connector found\n");
        drmModeFreeResources(resources);
        close(fd);
        return 1;
    }
    
    // 查找平面
    plane_resources = drmModeGetPlaneResources(fd);
    if (!plane_resources) {
        printf("Failed to get plane resources\n");
        drmModeFreeResources(resources);
        close(fd);
        return 1;
    }
    
    printf("Found %u planes\n", plane_resources->count_planes);
    
    // 尝试不同的平面类型
    for (uint32_t type = 0; type < 3; type++) {
        const char *type_str;
        switch (type) {
            case DRM_PLANE_TYPE_OVERLAY:
                type_str = "OVERLAY";
                break;
            case DRM_PLANE_TYPE_CURSOR:
                type_str = "CURSOR";
                break;
            case DRM_PLANE_TYPE_PRIMARY:
                type_str = "PRIMARY";
                break;
            default:
                type_str = "UNKNOWN";
        }
        
        printf("Trying %s planes...\n", type_str);
        
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
                            if (props->prop_values[j] == type) {
                                plane_id = plane->plane_id;
                                printf("Found %s plane: %u\n", type_str, plane_id);
                                drmModeFreeProperty(prop);
                                drmModeFreeObjectProperties(props);
                                drmModeFreePlane(plane);
                                goto plane_found;
                            }
                        }
                        drmModeFreeProperty(prop);
                    }
                }
                drmModeFreeObjectProperties(props);
            }
            drmModeFreePlane(plane);
        }
    }
    
    printf("No plane found\n");
    drmModeFreePlaneResources(plane_resources);
    drmModeFreeResources(resources);
    close(fd);
    return 1;
    
plane_found:
    drmModeFreePlaneResources(plane_resources);
    
    // 创建 Dumb Buffer
    struct drm_mode_create_dumb creq = {0};
    struct drm_mode_map_dumb mreq = {0};
    
    creq.width = width;
    creq.height = height;
    creq.bpp = 32;
    
    if (drmIoctl(fd, DRM_IOCTL_MODE_CREATE_DUMB, &creq)) {
        perror("Failed to create dumb buffer");
        drmModeFreeResources(resources);
        close(fd);
        return 1;
    }
    
    printf("Created dumb buffer: handle=%u, pitch=%u, size=%llu\n", 
           creq.handle, creq.pitch, creq.size);
    
    // 创建 Framebuffer
    uint32_t handles[4] = {creq.handle};
    uint32_t pitches[4] = {creq.pitch};
    uint32_t offsets[4] = {0};
    
    if (drmModeAddFB2(fd, width, height, DRM_FORMAT_ARGB8888,
                      handles, pitches, offsets, &fb_id, 0)) {
        perror("Failed to create framebuffer");
        drmModeFreeResources(resources);
        close(fd);
        return 1;
    }
    
    printf("Created framebuffer: %u\n", fb_id);
    
    // 映射到用户空间
    mreq.handle = creq.handle;
    if (drmIoctl(fd, DRM_IOCTL_MODE_MAP_DUMB, &mreq)) {
        perror("Failed to map dumb buffer");
        drmModeFreeResources(resources);
        close(fd);
        return 1;
    }
    
    mapped = mmap(0, creq.size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, mreq.offset);
    if (mapped == MAP_FAILED) {
        perror("Failed to mmap buffer");
        drmModeFreeResources(resources);
        close(fd);
        return 1;
    }
    
    size = creq.size;
    stride = creq.pitch;
    
    printf("Mapped buffer at %p, size=%u, stride=%u\n", mapped, size, stride);
    
    // 填充缓冲区 (红色)
    memset(mapped, 0x00, size);  // 清零
    uint32_t *pixels = (uint32_t *)mapped;
    for (uint32_t i = 0; i < (width * height); i++) {
        pixels[i] = 0xFFFF0000;  // ARGB: 红色，不透明
    }
    
    printf("Filled buffer with red color\n");
    
    // 尝试设置平面
    printf("Attempting to set plane...\n");
    
    int ret = drmModeSetPlane(fd, plane_id, crtc_id, fb_id, 0,
                              100, 100, width, height,
                              0, 0, width << 16, height << 16);
    
    if (ret) {
        printf("drmModeSetPlane failed: %s (errno=%d)\n", strerror(errno), errno);
        
        // 尝试使用 drmModeSetCrtc 作为备选
        printf("Trying drmModeSetCrtc...\n");
        ret = drmModeSetCrtc(fd, crtc_id, fb_id, 0, 0, &connector_id, 1, NULL);
        if (ret) {
            printf("drmModeSetCrtc also failed: %s (errno=%d)\n", strerror(errno), errno);
        } else {
            printf("drmModeSetCrtc succeeded\n");
        }
    } else {
        printf("drmModeSetPlane succeeded\n");
    }
    
    // 等待一段时间让用户看到显示
    printf("Displaying for 3 seconds...\n");
    sleep(3);
    
    // 清理
    munmap(mapped, size);
    drmModeRmFB(fd, fb_id);
    drmModeFreeResources(resources);
    close(fd);
    
    printf("Test completed\n");
    return 0;
}