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
#include "quantum.h"
#include "lib/keyball/keyball.h" // KEYBALL_SAFE_RANGE

// このキーマップ独自のキーコード定義。
// KEYBALL_SAFE_RANGE(=QK_KB_16)を起点に、QMKのキーコード空間と衝突しないようにする。
enum custom_keycodes {
    CPI_x3 = KEYBALL_SAFE_RANGE,
    JPUS_TOG,
    MMD_TOG,
    MMT_DEC,
    MMT_INC,
    LIGHT_TOG,
    LIGHT_VAI,
    LIGHT_VAD,
    HUE_UP,
    SELWORD_LEFT,
    SELWORD_RIGHT,
    SELLINE_UP,
    SELLINE_DOWN,
    SPD_THL_UP,
    SPD_THL_DN,
    SPD_THU_UP,
    SPD_THU_DN,
    SPD_SCL_UP,
    SPD_SCL_DN,
};

// このキーボード独自のユーザー定義キーコードを処理する(押下時のみ)。
// process_record_user から呼ぶこと。
// 戻り値: true=以降の処理を継続, false=ここで処理済みとして打ち切り
bool process_user_keycode(uint16_t keycode, keyrecord_t *record);

// ユーザー定義キーコードを処理するためのディスパッチ関数。
// virtual_key.c の fire_virtual_key() から、0x7E00以降のキーコードに対して呼ばれる。
bool process_user_virtual_keycode(uint16_t keycode);
