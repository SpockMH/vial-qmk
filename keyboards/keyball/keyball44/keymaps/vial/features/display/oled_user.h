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
 
bool oled_task_user_func(void);
void oled_update(void);

// 右手OLED状態同期用のRPCハンドラを登録する。
// keymap.c の keyboard_post_init_user() から無条件で呼ぶこと
// (左右どちらがマスターになっても受信側として機能する必要があるため)。
void oled_status_sync_register(void);

// マスター側で状態変化を検知し、必要ならRPC送信する。
// keymap.c の housekeeping_task_user() から無条件で呼ぶこと。
// OLEDハードウェアの有無に関係なく左右どちらのMCUでも確実に実行される
// housekeeping_task_user() 経由にすることで、OLEDを持たない側がマスターに
// なった場合でも同期が止まらないようにしている。
void oled_status_sync_task(void);