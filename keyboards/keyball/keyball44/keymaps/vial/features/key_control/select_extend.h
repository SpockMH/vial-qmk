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

#ifdef __cplusplus
extern "C" {
#endif

// レイアウト上のキーから使う場合はkeymap.c側で紐付け:
//   uint16_t SELECT_WORD_LEFT_KEYCODE  = SELWORD_LELT;
//   uint16_t SELECT_WORD_RIGHT_KEYCODE = SELWORD_RIGHT;
//   uint16_t SELECT_LINE_UP_KEYCODE    = SELLINE_UP;
//   uint16_t SELECT_LINE_DOWN_KEYCODE  = SELLINE_DOWN;
extern uint16_t SELECT_WORD_LEFT_KEYCODE;
extern uint16_t SELECT_WORD_RIGHT_KEYCODE;
extern uint16_t SELECT_LINE_UP_KEYCODE;
extern uint16_t SELECT_LINE_DOWN_KEYCODE;

// 直接呼び出し用(gesture_layerなどキーコードを経由しない箇所から呼ぶ)
void select_extend_word_left(void);
void select_extend_word_right(void);
void select_extend_line_up(void);
void select_extend_line_down(void);

// 選択モードを強制解除したい場合に呼ぶ
void select_extend_reset(void);

// process_record_user()内で呼ぶ(レイアウト上のキー用)
bool process_select_extend(uint16_t keycode, keyrecord_t* record);

#ifdef __cplusplus
}
#endif