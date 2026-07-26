#include "virtual_key.h"
#include "quantum.h"

void fire_virtual_key(uint8_t row, uint8_t col) {
    keypos_t pos      = {.row = row, .col = col};
    uint8_t  layer     = get_highest_layer(layer_state);
    uint16_t assigned  = keymap_key_to_keycode(layer, pos);

    if (assigned == KC_NO || assigned == KC_TRNS) {
        return;
    }

    // 0x7E00以降はQK_KB系/ユーザー定義キーコード。
    // process_record_userを経由しないこの経路ではtap_code16()が正しく
    // 機能しないため、keymap.c側のディスパッチ関数を直接叩く。
    if (assigned >= 0x5200) {
        process_user_virtual_keycode(assigned);
        return;
    }

    tap_code16(assigned);
}