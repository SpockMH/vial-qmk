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