#pragma once

#include <stdint.h>

// EECONFIG_USER_DATA_SIZE(config.hで16を指定)に合わせたサイズ。
// フィールドはサイズの大きい順に並べ、パディングが余計に入らないようにしてある。
// 合計: 2+2+1+1+1+9(予備) = 16byte
typedef struct {
    int16_t speed_lower_threshold; // mouse_speed_smoothing: 下限閾値
    int16_t speed_upper_threshold; // mouse_speed_smoothing: 上限閾値
    uint8_t speed_min_scale_pct;   // mouse_speed_smoothing: 下限側の最小倍率(%)
    uint8_t m_term;
    uint8_t hue;
    uint8_t reserved[9];           // 将来の拡張用(現在未使用)
} user_config_t;

void user_config_init(void);
void save_user_config(void);