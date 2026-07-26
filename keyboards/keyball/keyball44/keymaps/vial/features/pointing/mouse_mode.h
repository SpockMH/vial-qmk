#pragma once
#include "quantum.h"

// ===== key処理（process_record_user から呼ぶ）=====
bool mouse_mode_process(uint16_t keycode, keyrecord_t *record);

// ===== matrix_scan_user から呼ぶ =====
bool mouse_mode_scan(void);

void clear_m_buf(void);
// ===== 現在設定値取得 =====
bool get_m_mode(void);
uint8_t mouse_mode_get_term(void);
void mouse_mode_set_term(uint8_t input);

void mouse_move(bool move);

// ★新しく外部から呼び出せるようにする関数
void mouse_mode_toggle(void);
void mouse_mode_change_term(bool is_increment);