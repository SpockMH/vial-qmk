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

#include "mouse2key.h"
#include "timer.h"
#include "action.h"
#include "lib/keyball/keyball.h"

// 一定量動くたびに1回キーを発火させるための閾値
#ifndef MOUSE2KEY_MOVE_THRESHOLD
#    define MOUSE2KEY_MOVE_THRESHOLD 10
#endif

// 垂直方向に発火した後、横方向の発火を抑制する時間[ms]
// (斜め移動時に上下キーと左右キーが同時に暴発するのを防ぐ)
#ifndef MOUSE2KEY_VERTICAL_LOCK_MS
#    define MOUSE2KEY_VERTICAL_LOCK_MS 400
#endif

static int16_t  tension_x = 0;
static int16_t  tension_y = 0;
static uint32_t vertical_fired_at = 0;

void process_mouse2key_reset(void) {
    tension_x = 0;
    tension_y = 0;
    vertical_fired_at = 0;
}

void process_mouse2key(report_mouse_t *r, uint16_t up, uint16_t down, uint16_t left, uint16_t right) {
    int16_t x = r->x;
    int16_t y = r->y;

    if (abs(y) >= abs(x)){
        tension_y += y / keyball_get_cpi();
        // ---- 垂直方向 (up/down) ----
        if (tension_y > MOUSE2KEY_MOVE_THRESHOLD) {
            if (down != KC_NO) tap_code16(down);
            tension_y = 0;
            tension_x = 0; // 垂直発火時は水平テンションもクリア(斜め誤動作防止)
            vertical_fired_at = timer_read32();
        } else if (tension_y < -MOUSE2KEY_MOVE_THRESHOLD) {
            if (up != KC_NO) tap_code16(up);
            tension_y = 0;
            tension_x = 0;
            vertical_fired_at = timer_read32();
        }
    } else {
        tension_x += x / keyball_get_cpi();
        // ---- 水平方向 (left/right) ----
        // 垂直発火直後はロック時間内なら水平発火を抑制する
        if (TIMER_DIFF_32(timer_read32(), vertical_fired_at) < MOUSE2KEY_VERTICAL_LOCK_MS) {
            tension_x = 0;
            r->x = 0;
            r->y = 0;
            return;
        }

        if (tension_x > MOUSE2KEY_MOVE_THRESHOLD) {
            if (right != KC_NO) tap_code16(right);
            tension_x = 0;
        } else if (tension_x < -MOUSE2KEY_MOVE_THRESHOLD) {
            if (left != KC_NO) tap_code16(left);
            tension_x = 0;
        }
        tension_y += (tension_y > 0 ? -2 : 2);
    }

    r->x = 0;
    r->y = 0;
}