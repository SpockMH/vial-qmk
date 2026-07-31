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
#include "quantum.h"

/**
 * process_mouse2key
 *
 * トラックボールの動き(r->x, r->y)をテンション式の方向キー入力に変換する。
 * 一定量動くたびに up/down/left/right のいずれかのキーコードを1回 tap する。
 * 処理後、r->x/r->y は関数内で 0 クリアされる(呼び出し側でのクリアは不要)。
 *
 * 呼び出し側(pointing_device_task_user等)の責務:
 *   - どのレイヤー/モードでこの関数を呼ぶか判定する
 *   - 渡すキーコードを決める (KC_NO を渡せばそのキーは常に無効)
 *
 * この関数の責務:
 *   - x/y の動き量をテンションとして蓄積する
 *   - テンションが閾値を超えたら対応するキーコードを1回 tap する
 *   - 縦方向に動いた直後は一定時間横方向の発火を抑制する(誤操作防止)
 *   - 処理後に r->x/r->y を 0 クリアする
 *
 * @param r      マウスレポート(x/yを読み取り、末尾で0クリアする)
 * @param up     上方向で発火するキーコード (KC_NO で無効)
 * @param down   下方向で発火するキーコード (KC_NO で無効)
 * @param left   左方向で発火するキーコード (KC_NO で無効)
 * @param right  右方向で発火するキーコード (KC_NO で無効)
 */
void process_mouse2key(report_mouse_t *r, uint16_t up, uint16_t down, uint16_t left, uint16_t right);

/**
 * process_mouse2key の内部テンション/タイマー状態をリセットする。
 * レイヤー切替時や az1uball モード切替時に呼ぶこと。
 */
void process_mouse2key_reset(void);