#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <xf86drm.h>
#include <xf86drmMode.h>
#include <drm_fourcc.h>

int main() {
    int fd;
    drmModeRes *resources;
    drmModePlaneRes *plane_resources;
    
    // 打开 DRM 设备
    fd = open("/dev/dri/card1", O_RDWR);
    if (fd < 0) {
        perror("Failed to open DRM device");
        return 1;
    }
    
    printf("DRM device opened successfully\n");
    
    // 获取资源
    resources = drmModeGetResources(fd);
    if (!resources) {
        perror("Failed to get DRM resources");
        close(fd);
        return 1;
    }
    
    printf("Connectors: %d\n", resources->count_connectors);
    printf("CRTCs: %d\n", resources->count_crtcs);
    printf("Encoders: %d\n", resources->count_encoders);
    printf("Framebuffers: %d\n", resources->count_fbs);
    
    // 启用通用平面能力
    if (drmSetClientCap(fd, DRM_CLIENT_CAP_UNIVERSAL_PLANES, 1)) {
        printf("Failed to set universal planes capability\n");
    } else {
        printf("Universal planes capability enabled\n");
    }
    
    // 获取平面资源
    plane_resources = drmModeGetPlaneResources(fd);
    if (!plane_resources) {
        printf("Failed to get plane resources\n");
    } else {
        printf("Planes: %d\n", plane_resources->count_planes);
        
        // 遍历所有平面
        for (uint32_t i = 0; i < plane_resources->count_planes; i++) {
            drmModePlane *plane = drmModeGetPlane(fd, plane_resources->planes[i]);
            if (plane) {
                printf("Plane %d: ID=%d, CRTC_ID=%d, FB_ID=%d\n", 
                       i, plane->plane_id, plane->crtc_id, plane->fb_id);
                
                // 获取平面类型
                drmModeObjectProperties *props = drmModeObjectGetProperties(fd, 
                    plane->plane_id, DRM_MODE_OBJECT_PLANE);
                if (props) {
                    for (uint32_t j = 0; j < props->count_props; j++) {
                        drmModePropertyPtr prop = drmModeGetProperty(fd, props->props[j]);
                        if (prop) {
                            if (strcmp(prop->name, "type") == 0) {
                                const char *type_str;
                                switch (props->prop_values[j]) {
                                    case DRM_PLANE_TYPE_OVERLAY:
                                        type_str = "OVERLAY";
                                        break;
                                    case DRM_PLANE_TYPE_PRIMARY:
                                        type_str = "PRIMARY";
                                        break;
                                    case DRM_PLANE_TYPE_CURSOR:
                                        type_str = "CURSOR";
                                        break;
                                    default:
                                        type_str = "UNKNOWN";
                                }
                                printf("  Type: %s (%lu)\n", type_str, props->prop_values[j]);
                            }
                            drmModeFreeProperty(prop);
                        }
                    }
                    drmModeFreeObjectProperties(props);
                }
                drmModeFreePlane(plane);
            }
        }
        drmModeFreePlaneResources(plane_resources);
    }
    
    // 检查连接器状态
    for (int i = 0; i < resources->count_connectors; i++) {
        drmModeConnector *connector = drmModeGetConnector(fd, resources->connectors[i]);
        if (connector) {
            printf("Connector %d: ID=%d, Status=%s\n", 
                   i, connector->connector_id,
                   connector->connection == DRM_MODE_CONNECTED ? "CONNECTED" : "DISCONNECTED");
            
            if (connector->connection == DRM_MODE_CONNECTED) {
                printf("  Modes: %d\n", connector->count_modes);
                if (connector->count_modes > 0) {
                    printf("  Preferred mode: %dx%d\n", 
                           connector->modes[0].hdisplay, 
                           connector->modes[0].vdisplay);
                }
            }
            drmModeFreeConnector(connector);
        }
    }
    
    drmModeFreeResources(resources);
    close(fd);
    
    printf("\nDRM test completed successfully\n");
    return 0;
}
