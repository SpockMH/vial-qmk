#include "az1uball_gesture.h"
#include "timer.h"
#include "action.h"
#include "virtual_key.h"

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


void process_az1uball_gesture(int16_t x, int16_t y, bool click) {
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