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

#include "eeconfig_user.h"
#include "eeconfig.h"
#include "features/pointing/mouse_mode.h"
#include "features/lighting/rgblight_user.h"
#include "features/pointing/mouse_speed_smoothing.h"

static user_config_t user_config = {
    .speed_lower_threshold = 10,
    .speed_upper_threshold = 40,
    .speed_min_scale_pct   = 50,
    .m_term                = 60,
    .hue                   = 147,
    .reserved              = {0},
};

void save_user_config(void) {
    user_config.m_term = mouse_mode_get_term();
    user_config.hue    = get_hue();
    mouse_speed_smoothing_get_config(&user_config.speed_lower_threshold, &user_config.speed_upper_threshold, &user_config.speed_min_scale_pct);

    eeconfig_update_user_datablock(&user_config);
}

void user_config_init(void) {
    if (!eeconfig_is_user_datablock_valid()) {
        // 初回起動時など、まだ有効なデータが無い場合はデフォルト値のまま書き込んでおく
        eeconfig_init_user_datablock();
        eeconfig_update_user_datablock(&user_config);
    } else {
        eeconfig_read_user_datablock(&user_config);
    }

    mouse_mode_set_term(user_config.m_term);
    set_hue(user_config.hue);
    mouse_speed_smoothing_set_config(user_config.speed_lower_threshold, user_config.speed_upper_threshold, user_config.speed_min_scale_pct);
}