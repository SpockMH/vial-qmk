#pragma once
#include "quantum.h"

// CPI_x3を押している間だけCPIを3倍にする処理。
// process_record_user から呼ぶこと。
// 戻り値: true=以降の処理を継続, false=ここで処理済みとして打ち切り
bool process_cpi_x3(uint16_t keycode, keyrecord_t *record);

// 日本語入力(IME)のかな/英数状態に追従して、変換確定後に自動でIME ON(かな)へ戻す処理。
// KC_LEFT_BRACKETでIME ONを明示的に叩いた直後の状態もここでまとめて管理する。
// process_record_user から呼ぶこと。
// 戻り値: true=以降の処理を継続, false=ここで処理済みとして打ち切り
bool process_kana_ime(uint16_t keycode, keyrecord_t *record);
