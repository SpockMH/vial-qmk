/**
 * Copyright (c) 2026 SpockMH
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACTText, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

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