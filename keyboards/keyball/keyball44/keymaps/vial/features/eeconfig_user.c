#include "eeconfig_user.h"
#include "eeconfig.h"
#include "mouse_mode.h"
#include "rgblight_user.h"
#include "features/mouse_speed_smoothing.h"

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