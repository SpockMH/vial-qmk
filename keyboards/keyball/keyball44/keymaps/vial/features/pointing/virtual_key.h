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

// keymap.c側で定義する、ユーザー定義キーコード(0x7E00以降)のディスパッチ関数。
// switch文で個々のキーコードに対応する関数を直接呼び出す。
// AZ1UBALLのdpad/gesture等、process_record_userを経由しない経路から
// virtual_key.c経由で呼ばれる。
bool process_user_virtual_keycode(uint16_t keycode);

// AZ1UBALLのdpad/gesture機能から、仮想マトリクス位置(row,col)に割り当てられた
// キーを1回分「実行」する。
//   - 通常のキーコード(KC_A等)          → tap_code16()でHID出力する
//   - ユーザー定義キーコード(0x7E00以降) → process_user_virtual_keycode()に委譲し、
//                                          対応する関数を直接呼び出す
void fire_virtual_key(uint8_t row, uint8_t col);