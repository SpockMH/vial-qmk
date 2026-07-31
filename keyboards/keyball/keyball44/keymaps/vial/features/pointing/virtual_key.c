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