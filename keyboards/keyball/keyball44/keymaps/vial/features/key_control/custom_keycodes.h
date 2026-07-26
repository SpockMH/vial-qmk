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
    SPL_TOG,
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
