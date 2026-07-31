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

// CPI_x3を押している間だけCPIを3倍にする処理。
// process_record_user から呼ぶこと。
// 戻り値: true=以降の処理を継続, false=ここで処理済みとして打ち切り
bool process_cpi_x3(uint16_t keycode, keyrecord_t *record);

// 日本語入力(IME)のかな/英数状態に追従して、変換確定後に自動でIME ON(かな)へ戻す処理。
// KC_LEFT_BRACKETでIME ONを明示的に叩いた直後の状態もここでまとめて管理する。
// process_record_user から呼ぶこと。
// 戻り値: true=以降の処理を継続, false=ここで処理済みとして打ち切り
bool process_kana_ime(uint16_t keycode, keyrecord_t *record);
