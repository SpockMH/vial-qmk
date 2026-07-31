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
#include <stdint.h>
#include <stdbool.h>
#include "quantum.h"
 
// 十字キー4方向用の仮想マトリクス位置(info.json上の未使用スロット)。
// Vialのキーマップ編集画面から、この4箇所に自由にキーを割り当てられる。
#define AZ_DPAD_UP_ROW    3
#define AZ_DPAD_UP_COL    0
#define AZ_DPAD_DOWN_ROW  7
#define AZ_DPAD_DOWN_COL  0
#define AZ_DPAD_LEFT_ROW  7
#define AZ_DPAD_LEFT_COL  2
#define AZ_DPAD_RIGHT_ROW 7
#define AZ_DPAD_RIGHT_COL 3
#define AZ_DPAD_CLICK_ROW 3
#define AZ_DPAD_CLICK_COL 6

// AZ1UBALLのボタンエッジ検出とジェスチャー入力への振り分けをまとめる。
// scroll_mode時はr->x/yを、それ以外はr->h/-r->vをジェスチャー入力として使い、
// 使用した軸はマウスレポートから0クリアする。
void handle_az1uball_input(report_mouse_t *r, bool scroll_mode);