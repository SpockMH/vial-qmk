#include "arrow_layer.h"
#include "timer.h"
#include "action.h"
#include "lib/keyball/keyball.h"

// 一定量動くたびに1回キーを発火させるための閾値
#ifndef ARROW_LAYER_MOVE_THRESHOLD
#    define ARROW_LAYER_MOVE_THRESHOLD 10
#endif

// 垂直方向に発火した後、横方向の発火を抑制する時間[ms]
// (斜め移動時に上下キーと左右キーが同時に暴発するのを防ぐ)
#ifndef ARROW_LAYER_VERTICAL_LOCK_MS
#    define ARROW_LAYER_VERTICAL_LOCK_MS 400
#endif

static int16_t  tension_x = 0;
static int16_t  tension_y = 0;
static uint32_t vertical_fired_at = 0;

void process_arrow_layer_reset(void) {
    tension_x = 0;
    tension_y = 0;
    vertical_fired_at = 0;
}

void process_arrow_layer(int16_t x, int16_t y, uint16_t up, uint16_t down, uint16_t left, uint16_t right) {
    
    if (abs(y) >= abs(x)){
        tension_y += y / keyball_get_cpi();
        // ---- 垂直方向 (up/down) ----
        if (tension_y > ARROW_LAYER_MOVE_THRESHOLD) {
            if (down != KC_NO) tap_code16(down);
            tension_y = 0;
            tension_x = 0; // 垂直発火時は水平テンションもクリア(斜め誤動作防止)
            vertical_fired_at = timer_read32();
        } else if (tension_y < -ARROW_LAYER_MOVE_THRESHOLD) {
            if (up != KC_NO) tap_code16(up);
            tension_y = 0;
            tension_x = 0;
            vertical_fired_at = timer_read32();
        }
    } else {
        tension_x += x / keyball_get_cpi();
        // ---- 水平方向 (left/right) ----
        // 垂直発火直後はロック時間内なら水平発火を抑制する
        if (TIMER_DIFF_32(timer_read32(), vertical_fired_at) < ARROW_LAYER_VERTICAL_LOCK_MS) {
            tension_x = 0;
            return;
        }

        if (tension_x > ARROW_LAYER_MOVE_THRESHOLD) {
            if (right != KC_NO) tap_code16(right);
            tension_x = 0;
        } else if (tension_x < -ARROW_LAYER_MOVE_THRESHOLD) {
            if (left != KC_NO) tap_code16(left);
            tension_x = 0;
        }
        tension_y += (tension_y > 0 ? -2 : 2);
    }
}