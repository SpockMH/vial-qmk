#include "eeconfig_user.h"
#include "eeconfig.h"
#include "mouse_mode.h"
#include "rgblight_user.h"

static user_config_t user_config = {
    .m_term = 60,
    .hue = 147,
    .Reserved1 = 0,
    .Reserved2 = 0
};

void save_user_config(void){
    user_config.m_term = mouse_mode_get_term();
    user_config.hue = get_hue();
    eeconfig_update_user(user_config.raw);
}

void user_config_init(void){
    user_config.raw = eeconfig_read_user();
    mouse_mode_set_term(user_config.m_term);
    set_hue(user_config.hue);

}
