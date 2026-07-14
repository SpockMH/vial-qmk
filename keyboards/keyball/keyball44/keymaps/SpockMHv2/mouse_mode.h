#pragma once
#include "quantum.h"

// ===== 初期化 =====
void mouse_mode_init(void);

// ===== key処理（process_record_user から呼ぶ）=====
bool mouse_mode_process(uint16_t keycode, keyrecord_t *record);

// ===== matrix_scan_user から呼ぶ =====
bool mouse_mode_scan(void);

void clear_m_buf(void);
// ===== 現在設定値取得 =====
bool get_m_mode(void);
uint8_t mouse_mode_get_term(void);

void mouse_move(bool move);
uint8_t get_hue(void);

#ifndef BALL
uint8_t get_hi_score(void);
void set_hi_score(uint8_t score);
#endif