#ifndef LINUXHUD_H
#define LINUXHUD_H

/**
 * @file linuxhud.h
 * @brief LinuxHUD — 硬件级平视显示器
 *
 * 绕过图形栈，通过 DRM/KMS 直接在显卡叠加平面上渲染信息。
 *
 * 快速上手:
 * @code
 *   linuxhud_config_t cfg;
 *   linuxhud_config_defaults(&cfg);
 *   cfg.x = 50;  cfg.y = 50;
 *   cfg.width = 400;  cfg.height = 120;
 *
 *   linuxhud_t *hud = NULL;
 *   linuxhud_create(&hud, &cfg);
 *
 *   linuxhud_text_style_t style = {
 *       .color = LINUXHUD_COLOR_GREEN,
 *       .font  = { .family = "Sans", .size = 28 },
 *       .align = LINUXHUD_ALIGN_LEFT,
 *   };
 *   linuxhud_draw_text(hud, "Hello LinuxHUD!", &style);
 *
 *   // ... 保持显示 ...
 *
 *   linuxhud_destroy(hud);
 * @endcode
 */

#include "linuxhud/types.h"
#include "linuxhud/drm.h"
#include "linuxhud/render.h"
#include "linuxhud/widget.h"
#include "linuxhud/hud.h"

#endif /* LINUXHUD_H */
