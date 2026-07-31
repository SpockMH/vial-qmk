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

#include "az1uball_gesture.h"
#include "timer.h"
#include "action.h"
#include "virtual_key.h"
#include "features/lighting/lighting_tracking.h"

// ============================================================
// 動作イメージ:
//
//   IDLE --(閾値超えの動き)--> MOVING --(50ms経過)--> LOCKED(発火)
//                                                        │
//                                          (10ms無入力検知)
//                                                        ▼
//                                                      IDLE
//
//   ※ LOCKED中に新たな動きが来た場合は、その都度停止検知タイマー
//     (last_motion_time)を更新するだけで、発火は50ms時点の1回のみ。
//   ※ LOCKED中に10ms無入力を検知した時点で即座にIDLEへ戻る
//     (COOLDOWN的な固定待ち時間は設けない。次の動きが来れば
//      IDLE側の閾値判定からそのままMOVINGへ入れるため)。
// ============================================================

// MOVINGに入るための最小移動量(ノイズ除去用の下限)
#ifndef GESTURE_START_THRESHOLD
#    define GESTURE_START_THRESHOLD 0
#endif

// 動き出しから発火判定を行うまでの時間[ms]
#ifndef GESTURE_FIRE_DELAY_MS
#    define GESTURE_FIRE_DELAY_MS 60
#endif

// 発火後、「止まった」とみなしIDLEへ戻すまでの無入力時間[ms]
#ifndef GESTURE_STOP_DETECT_MS
#    define GESTURE_STOP_DETECT_MS 30
#endif

// 発火判定に使う最小テンション(50ms経過時点でこれ未満なら発火しない)
#ifndef GESTURE_FIRE_THRESHOLD
#    define GESTURE_FIRE_THRESHOLD 2
#endif

typedef enum {
    GESTURE_STATE_IDLE = 0,
    GESTURE_STATE_MOVING,
    GESTURE_STATE_LOCKED,
} gesture_state_t;

static gesture_state_t state = GESTURE_STATE_IDLE;
static int16_t  tension_x = 0;
static int16_t  tension_y = 0;
static uint32_t motion_start_time = 0; // MOVING開始時刻(50ms計測の基準)
static uint32_t last_motion_time  = 0; // 直近の動き検出時刻(10ms停止検知の基準)

static void start_new_gesture(int16_t x, int16_t y, uint32_t now) {
    tension_x = x;
    tension_y = y;
    motion_start_time = now;
    last_motion_time  = now;
    state = GESTURE_STATE_MOVING;
}

static void gesture_fire_key(void) {
    int16_t ax = tension_x < 0 ? -tension_x : tension_x;
    int16_t ay = tension_y < 0 ? -tension_y : tension_y;

    if (ax < GESTURE_FIRE_THRESHOLD && ay < GESTURE_FIRE_THRESHOLD) {
        return; // 閾値未満はノイズとして無視(発火しない)
    }

    if (ax >= ay) {
        if (tension_x > 0){
            //on_right();
            fire_virtual_key(AZ_DPAD_RIGHT_ROW,AZ_DPAD_RIGHT_COL);
        } else {
            //on_left();
            fire_virtual_key(AZ_DPAD_LEFT_ROW,AZ_DPAD_LEFT_COL);
        }
    } else {
        if (tension_y > 0) {
            //on_down();
            fire_virtual_key(AZ_DPAD_DOWN_ROW,AZ_DPAD_DOWN_COL);
        } else {
            //on_up();
            fire_virtual_key(AZ_DPAD_UP_ROW,AZ_DPAD_UP_COL);
        }
    }
}


static void process_az1uball_gesture(int16_t x, int16_t y, bool click) {
    bool     has_motion = (x != 0 || y != 0);
    uint32_t now        = timer_read32();
    
    if (click) {
        fire_virtual_key(AZ_DPAD_CLICK_ROW,AZ_DPAD_CLICK_COL);
        return;
    }

    switch (state) {
        case GESTURE_STATE_IDLE:
            if (has_motion) {
                int16_t ax = x < 0 ? -x : x;
                int16_t ay = y < 0 ? -y : y;
                if (ax >= GESTURE_START_THRESHOLD || ay >= GESTURE_START_THRESHOLD) {
                    start_new_gesture(x, y, now);
                }
            }
            break;

        case GESTURE_STATE_MOVING:
            if (has_motion) {
                tension_x += x;
                tension_y += y;
                last_motion_time = now;
            }
            // 動いていようが止まっていようが、50ms経過したら判定・発火
            if (TIMER_DIFF_32(now, motion_start_time) >= GESTURE_FIRE_DELAY_MS) {
                gesture_fire_key();
                state = GESTURE_STATE_LOCKED;
                last_motion_time = now; // ここから10ms停止検知を開始
            }
            break;

        case GESTURE_STATE_LOCKED:
            if (has_motion) {
                // まだ動いている間は停止検知の10msを満たさないよう更新し続ける
                // (発火は50ms時点の1回のみなので、ここで再発火はしない)
                last_motion_time = now;
            } else if (TIMER_DIFF_32(now, last_motion_time) >= GESTURE_STOP_DETECT_MS) {
                // 10ms無入力を検知 = 即座にIDLEへ戻り、次のジェスチャー受付可能にする
                tension_x = 0;
                tension_y = 0;
                state = GESTURE_STATE_IDLE;
            }
            break;
    }
}

// AZ1UBALLのボタンエッジ検出とジェスチャー入力への振り分けをまとめる。
// scroll_mode時はr->x/yを、それ以外はr->h/-r->vをジェスチャー入力として使い、
// 使用した軸はマウスレポートから0クリアする。
// (旧 keymap.c の handle_az1uball_input を移設)
void handle_az1uball_input(report_mouse_t *r, bool scroll_mode) {
    static bool az_btn_prev = false;
    bool az_btn_now    = (r->buttons & MOUSE_BTN6);
    bool az_press_edge = az_btn_now && !az_btn_prev;  // 押下エッジ検出

    if (az_press_edge) {
        lighting_tracking_set_position(2, 5);
        lighting_tracking_trigger(false, true);
    }

    int16_t h = scroll_mode ? r->x : r->h;
    int16_t v = scroll_mode ? r->y : -r->v;
    process_az1uball_gesture(h, v, az_press_edge);

    if (scroll_mode) { r->x = 0; r->y = 0; }
    else             { r->h = 0; r->v = 0; }

    az_btn_prev = az_btn_now;
}