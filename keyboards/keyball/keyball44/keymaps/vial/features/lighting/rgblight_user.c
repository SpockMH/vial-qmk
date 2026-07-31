/* Copyright 2016-2017 Yang Liu
 * Copyright (c) 2026 SpockMH
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

//* rgblight_user.c */
#include "quantum.h"
#include <stdint.h>
#include <stdbool.h>
#include "timer.h"
#include "ws2812.h"
#include "color.h"
#include "rgblight_user.h"
#include "lighting_tracking.h"
#include "transactions.h"
#include <lib/lib8tion/lib8tion.h>

#define RGBLIGHT_LIMIT_VAL  150
#define RGBLIGHT_VAL_STEP   50
#define RGB_IDLE_TIMEOUT_MS 30000

// 共有変数
rgblight_simple_config_t rgblight_config;
static uint16_t effect_timer;
static rgb_led_t led[RGBLED_NUM];

// アイドル管理用(無操作が続いたら自動消灯する)
static uint32_t last_rgb_activity = 0;
static bool     rgb_is_idle       = false;


/*  
//Keyball44配置図
    L00,L01,L02,L03,L04,L05,    R05,R04,R03,R02,R01,R00, \
    L10,L11,L12,L13,L14,L15,    R15,R14,R13,R12,R11,R10, \
    L20,L21,L22,L23,L24,L25,    R25,R24,R23,R22,R21,R20, \
        L31,L32,L33,L34,L35,    R35,R34,        R31      \
    ) 
    { 
        {   L00,   L01,   L02,   L03,   L04,   L05 }, \
        {   L10,   L11,   L12,   L13,   L14,   L15 }, \
        {   L20,   L21,   L22,   L23,   L24,   L25 }, \
        { KC_NO,   L31,   L32,   L33,   L34,   L35 }, \
        {   R00,   R01,   R02,   R03,   R04,   R05 }, \
        {   R10,   R11,   R12,   R13,   R14,   R15 }, \
        {   R20,   R21,   R22,   R23,   R24,   R25 }, \
        { KC_NO,   R31, KC_NO, KC_NO,   R34,   R35 }, \
    }
*/

/*
//Keyball44 LED No.(親指は下向きを採用)
    17, 14, 10, 6, 3, 0,     56, 53, 50, 47, 43, 40, 
    18, 15, 11, 7, 4, 1,     57, 54, 51, 48, 44, 41, 
    19, 16, 12, 8, 5, 2,     58, 55, 52, 49, 45, 42, 
        13,  9,27,28,29,     30, 31,      46     
*/
// LEDインデックスマップ
// led_index用の列数。物理キーはcol0〜5の6列のみで、col6はaz1uball仮想キーコードを
// vial側のキーマップ配列に割り当てるためだけに追加された列であり、対応する物理キー・LEDは無い。
// MATRIX_COLS(=7)をそのままled_index関連のループ境界に使うと、存在しないcol6のマスまで
// 距離計算やSWIRL/CROSSエフェクトの対象になってしまう(そのマスのled_indexは配列外を指すか、
// 意図せずled[0]やled[59]を誤って上書きしてしまう)。そのため専用の定数として分離し、
// led_index関連の全ループ(このファイル内)で必ずこちらを使うこと。
#define LED_MATRIX_COLS (MATRIX_COLS - 1)

const uint8_t led_index[MATRIX_ROWS][LED_MATRIX_COLS] = { 
    { 17, 14, 10,  6,  3,  0 },  // Row 0 left 
    { 18, 15, 11,  7,  4,  1 },  // Row 1 left
    { 19, 16, 12,  8,  5,  2 },  // Row 2 left
    { 59, 13,  9, 27, 28, 29 },  // Row 3 left
    { 40, 43, 47, 50, 53, 56 },  // Row 4 right
    { 41, 44, 48, 51, 54, 57 },  // Row 5 right
    { 42, 45, 49, 52, 55, 58 },  // Row 6 right
    { 59, 46, 59, 59, 31, 30 }   // Row 7 right
};

static uint8_t value[RGBLED_NUM]     = {0};
static uint8_t hue_value[RGBLED_NUM] = {0};

static uint8_t last_row = 0;
static uint8_t last_col = 0;

// Splash Wave定数
#define DIST_SCALE         10
#define WAVE_SOLID         100
#define WAVE_FADE          150
#define SPLASH_SPEED       2
#define SPLASH_MAX_RADIUS  (DIST_SCALE * 25)
#define SPLASH_LEAD_OFFSET 10
#define SPLASH_TIP_FALLOFF 15

// 花火風(タイピング時)の小さいスプラッシュ用パラメータ。
// MAX_RADIUSはWPMに応じて可変(下記splash_firework_radius_for_wpm参照)。
// WAVE_SOLID/WAVE_FADEは通常スプラッシュと同じ値にしてあり、実質的には
// MAX_RADIUS(広がる範囲の大きさ)だけが花火/通常で異なる形になる。
#define SPLASH_FIREWORK_MAX_RADIUS (DIST_SCALE * 1.5) // 直径2マス程度の局所的な広がりを狙った目安値(調整可)
#define SPLASH_FIREWORK_WAVE_SOLID 100
#define SPLASH_FIREWORK_WAVE_FADE  150

// WPMに応じて花火スプラッシュの最大到達半径を変化させる閾値。
//   〜SPLASH_FIREWORK_GROW_WPM_LOW                                   : SPLASH_FIREWORK_MAX_RADIUS固定(小さい花火)
//   SPLASH_FIREWORK_GROW_WPM_LOW〜SPLASH_FIREWORK_GROW_WPM_HIGH      : SPLASH_FIREWORK_MAX_RADIUS→SPLASH_MAX_RADIUSへ線形に拡大
//   SPLASH_FIREWORK_GROW_WPM_HIGH〜                                  : SPLASH_MAX_RADIUS固定(通常のクリック時スプラッシュと同じ大きさ)
#define SPLASH_FIREWORK_GROW_WPM_LOW  70
#define SPLASH_FIREWORK_GROW_WPM_HIGH 300

// Splash プール定数（数を変えるだけで同時発生数を調整可能）
#define SPLASH_MAX_COUNT 10

typedef struct {
    bool     active;
    uint8_t  center_row;
    uint8_t  center_col;
    uint8_t  start_hue;
    uint16_t radius;      // 現在の半径(毎フレームSPLASH_SPEEDずつ増加していく値)
    uint16_t max_radius;  // このスプラッシュの最大到達半径(発火した瞬間のWPMに応じて決定し、以後は固定)
    bool     firework;    // true: タイピング時の小さい「花火」風スプラッシュ(SPLASH_FIREWORK_WAVE_*を使う)
} splash_state_t;

static splash_state_t splash_pool[SPLASH_MAX_COUNT];
static uint8_t        splash_oldest = 0; // 次に上書きするスロットのインデックス（リングバッファ方式）

/* --- 内部ユーティリティ関数 --- */
static void sethsv_to_array(uint8_t hue, uint8_t sat, uint8_t val, rgb_led_t *target_led) {
    val = (val > RGBLIGHT_LIMIT_VAL) ? RGBLIGHT_LIMIT_VAL : val;
    HSV hsv = {hue, sat, val};
    RGB rgb = hsv_to_rgb(hsv);
    target_led->r = rgb.r;
    target_led->g = rgb.g;
    target_led->b = rgb.b;
}

static inline uint8_t keyball_distance(uint8_t r1, uint8_t c1, uint8_t r2, uint8_t c2) {
    if (r1 >= 4) { r1 -= 4; c1 = (5 - c1) + 8; }
    if (r2 >= 4) { r2 -= 4; c2 = (5 - c2) + 8; }

    int8_t dr = (int8_t)r1 - (int8_t)r2;
    int8_t dc = (int8_t)c1 - (int8_t)c2;
    
    if (dr < 0) dr = -dr;
    if (dc < 0) dc = -dc;

    // dr と dc の大小関係を特定
    uint8_t max_d = (dr > dc) ? dr : dc;
    uint8_t min_d = (dr > dc) ? dc : dr;

    // 1偏差 = 10 のスケールで直線距離（三平方の定理）を近似計算
    // Max + 0.375 * Min の整数演算（誤差最大約4%）
    uint16_t dist = (uint16_t)max_d * 10 + ((uint16_t)min_d * 38 / 100);

    return (dist > 255) ? 255 : (uint8_t)dist;
}

static void rgblight_value_reset(void) {
    for (uint8_t i = 0; i < RGBLED_NUM; i++) {
        value[i] = 0;
        hue_value[i] = rgblight_config.hue;
    }
}



// データを物理LEDへ送信する本質関数
static void rgblight_flush(void) {
    if (!rgblight_config.enable || rgblight_config.mode == RGBLIGHT_MODE_OFF) {
        for (uint8_t i = 0; i < RGBLED_NUM; i++) {
            led[i].r = 0; led[i].g = 0; led[i].b = 0;
        }
    }// 物理的に送信する個数（Keyball44なら片側30個）
    uint8_t send_count = LEFT_OFFSET; 

    if (is_keyboard_left()) {
        // ===================================================
        // 右手側（スレーブ）の送信処理
        // ===================================================
        // 配列 led[60] のうち、右手用のデータは「0〜29番目」に格納されています。
        // 先頭（&led[0]）からそのまま30個分を送信します。
        ws2812_setleds(&led[0], send_count);       
    } else {
        // ===================================================
        // 左手側（マスター）の送信処理
        // ===================================================
        // 配列 led[60] のうち、左手用のデータは「30〜59番目」に格納されています。
        // ポインタを &led[LEFT_OFFSET] にずらすことで、物理LEDには30番目のデータが「1個目のLED」として送信されます。
        ws2812_setleds(&led[LEFT_OFFSET], send_count);
    }
}

#define UNDERLED_START     20
#define UNDERLED_COUNT     20
#define UNDERLED_HUE_STEP  13

static void underled(bool on) {
    static uint8_t cur_hue = 0;
    uint8_t val = on ? rgblight_config.val : 0;
    for (uint8_t i = UNDERLED_START; i < UNDERLED_START + UNDERLED_COUNT; i++) {
        sethsv_to_array((i - UNDERLED_START) * UNDERLED_HUE_STEP + cur_hue, 255, val, &led[i]);
    }
    cur_hue++;
}

/* --- エフェクトロジック --- */
static void rgblight_effect_static(void) {
    for (uint8_t i = 0; i < RGBLED_NUM; i++) {
        sethsv_to_array(rgblight_config.hue, 255, rgblight_config.val, &led[i]);
    }
    rgblight_flush();
}

static void rgblight_effect_icewave(void) {
    uint8_t sat;
    static uint8_t cur_sat = 0;
    static bool flag = false;
    
    for (uint8_t i = 0; i < RGBLED_NUM; i++) {
        uint8_t led_sat = (255 / RGBLED_NUM * i);
        if(((led_sat + cur_sat * 2) / 256) % 2 == 0) {
            sat = led_sat + cur_sat * 2;
        } else {
            sat = 255 - (led_sat + cur_sat * 2);
        }
        sat = qadd8(sat, value[i]);
        sethsv_to_array(rgblight_config.hue, sat, rgblight_config.val, &led[i]);
    }
    underled(false);
    rgblight_flush();
    
    if (cur_sat == 255) flag = true;
    if (cur_sat == 0)   flag = false;
    if (flag) cur_sat--; else cur_sat++;
}

static void rgblight_effect_mousemove(void) {
    for (uint8_t i = 0; i < RGBLED_NUM; i++) {
        sethsv_to_array(hue_value[i], 255, value[i], &led[i]);
    }
    underled(true);
    rgblight_flush();
    rgblight_value(0, 0, false, false, false);
}

static void rgblight_effect_scrollmove(void) {
    for (uint8_t i = 0; i < RGBLED_NUM; i++) {
        sethsv_to_array(hue_value[i], 255, value[i], &led[i]);
    }
    underled(true);
    rgblight_flush();
    rgblight_value(0, 0, false, true, false);
}

void rgblight_effect_swirl(void) {
    uint8_t hue;
    static uint8_t current_hue = 0;
    uint8_t i, j;

    for (i = 0; i < LED_MATRIX_COLS; i++) {
        for (j = 0; j < MATRIX_ROWS; j++){
            hue = (8 * i + current_hue);
            sethsv_to_array(hue, 255, rgblight_config.val, &led[led_index[j][i]]);
        }
    }
    rgblight_flush();
    current_hue--;
}

void rgblight_effect_cross(void) {

    static uint8_t current_hue = 0;
    uint8_t i, j;

    for (i = 0; i < MATRIX_ROWS; i++) {
        for (j = 0; j < LED_MATRIX_COLS; j++) {
            // row と col の和で奇数・偶数を判定
            if ((i + j + (i == 3 ? 1:0)) % 2 != 0) {
                sethsv_to_array(current_hue, 255, rgblight_config.val, &led[led_index[i][j]]);
            } else {
                sethsv_to_array(current_hue + 128, 255, rgblight_config.val, &led[led_index[i][j]]);
            }
        }
    }
    
    underled(true);
    rgblight_flush();
    current_hue--;
}

/* --- 外部公開API関数 --- */
void rgblight_init(void) {
    lighting_register_rpc_handlers();

    ws2812_init();
    rgblight_config.enable = true;
    rgblight_config.mode   = RGBLIGHT_MODE_MOUSEMOVE;
    rgblight_config.sat    = 255;
    rgblight_config.val    = RGBLIGHT_LIMIT_VAL;
    rgblight_value_reset();
    effect_timer = timer_read();

    last_rgb_activity = timer_read32();
    rgb_is_idle        = false;
}

// キー入力等の操作があった際に呼ぶ。アイドル消灯中であれば再点灯する。
// (旧 keymap.c の wake_rgb を移設)
void rgblight_wake(void) {
    last_rgb_activity = timer_read32();
    if (rgb_is_idle) {
        rgblight_enable();
        rgb_is_idle = false;
    }
}

void rgblight_mode(uint8_t mode) {
    rgblight_config.mode = mode;
    rgblight_value_reset();
    if(is_keyboard_master()){ 
        transaction_rpc_send(USER_SYNC_LIGHTING, sizeof(rgblight_config), &rgblight_config);
    }
}

void rgblight_sethsv(uint8_t hue, uint8_t sat, uint8_t val) {
    rgblight_config.hue = hue; rgblight_config.sat = sat; rgblight_config.val = val;
}

bool rgblight_is_enabled(void) { return rgblight_config.enable; }
uint8_t rgblight_get_val(void)  { return rgblight_config.val; }


void rgblight_enable(void) {
    rgblight_config.enable = true;
    if(is_keyboard_master()){ 
        transaction_rpc_send(USER_SYNC_LIGHTING, sizeof(rgblight_config), &rgblight_config);
    }
}
void rgblight_disable(void) {
    rgblight_config.enable = false;
    if(is_keyboard_master()){ 
        transaction_rpc_send(USER_SYNC_LIGHTING, sizeof(rgblight_config), &rgblight_config);
    }
    rgblight_flush();
}
void rgblight_toggle(void) {
    rgblight_config.enable = !rgblight_config.enable;
    if(is_keyboard_master()){ 
        transaction_rpc_send(USER_SYNC_LIGHTING, sizeof(rgblight_config), &rgblight_config);
    }
    if (!rgblight_config.enable) { rgblight_flush(); }
}

void rgblight_increase_val(void) {
    uint8_t val = qadd8(rgblight_config.val, RGBLIGHT_VAL_STEP);
    rgblight_config.val = (val > RGBLIGHT_LIMIT_VAL) ? RGBLIGHT_LIMIT_VAL : val;
    if(is_keyboard_master()){ 
        transaction_rpc_send(USER_SYNC_LIGHTING, sizeof(rgblight_config), &rgblight_config);
    }
}

void rgblight_decrease_val(void) {
    rgblight_config.val = qsub8(rgblight_config.val, RGBLIGHT_VAL_STEP);
    if(is_keyboard_master()){ 
        transaction_rpc_send(USER_SYNC_LIGHTING, sizeof(rgblight_config), &rgblight_config);
    }
}

uint8_t get_hue(void){
    return rgblight_config.hue;
}

void set_hue(uint8_t value){
    rgblight_config.hue = value;
    if(is_keyboard_master()){ 
        transaction_rpc_send(USER_SYNC_LIGHTING, sizeof(rgblight_config), &rgblight_config);
    }
}


void rgblight_update_sync(rgblight_simple_config_t *master_config) {
    bool is_changed = false;
    // 1. ON/OFF状態に変化があるかチェック
    if (rgblight_config.enable != master_config->enable) {
        rgblight_config.enable = master_config->enable;
        if (!rgblight_config.enable) {
            rgblight_flush(); // オフになった瞬間は即時消灯
        }
        is_changed = true;
    }
    // 2. エフェクトモードに変化があるかチェック
    if (rgblight_config.mode != master_config->mode) {
        rgblight_mode(master_config->mode); // モード切り替えとバッファのリセット
        is_changed = true;
    }
    // 3. 色（HSV）に変化があるかチェック
    if (rgblight_config.hue != master_config->hue ||
        rgblight_config.sat != master_config->sat ||
        rgblight_config.val != master_config->val) {
        
        rgblight_sethsv(master_config->hue, master_config->sat, master_config->val);
        is_changed = true;
    }
    // 何かしらの変化があって、かつライトが有効な場合は即座にLEDへ反映
    if (is_changed && rgblight_config.enable) {
        rgblight_flush();
    }
}

// マスターからの USER_SYNC_LIGHTING RPC を受け取り rgblight_update_sync() に渡す。
// (旧 keymap.c の lighting_sync_slave_handler を移設)
void rgblight_sync_rpc_handler(uint8_t in_buflen, const void *in_data, uint8_t out_buflen, void *out_data) {
    rgblight_update_sync((rgblight_simple_config_t *)in_data);
}

// ライティング関連のRPCハンドラ(USER_SYNC_KEY_COUNTER / USER_SYNC_LIGHTING)を
// まとめて登録する。keymap.c の keyboard_post_init_user から呼ぶこと。
// (旧 keymap.c 側の2行の transaction_register_rpc() 呼び出しを移設)
void lighting_register_rpc_handlers(void) {
    transaction_register_rpc(USER_SYNC_KEY_COUNTER, lighting_tracking_rpc_handler);
    transaction_register_rpc(USER_SYNC_LIGHTING, rgblight_sync_rpc_handler);
}

// 空きスロットを探す。なければ splash_oldest を上書きしてそのインデックスを返す
static uint8_t splash_alloc(void) {
    for (uint8_t i = 0; i < SPLASH_MAX_COUNT; i++) {
        if (!splash_pool[i].active) return i;
    }
    // 全スロット使用中：最も古いスロット（リングバッファ）を再利用
    uint8_t slot = splash_oldest;
    splash_oldest = (splash_oldest + 1) % SPLASH_MAX_COUNT;
    return slot;
}

// WPMに応じて花火スプラッシュの最大到達半径を決める。
// SPLASH_FIREWORK_GROW_WPM_LOW〜HIGHの間は線形補間し、範囲外は両端の値で固定する。
static uint16_t splash_firework_radius_for_wpm(uint8_t wpm) {
    if (wpm <= SPLASH_FIREWORK_GROW_WPM_LOW) {
        return SPLASH_FIREWORK_MAX_RADIUS;
    }
    if (wpm >= SPLASH_FIREWORK_GROW_WPM_HIGH) {
        return SPLASH_MAX_RADIUS;
    }
    uint32_t span  = SPLASH_FIREWORK_GROW_WPM_HIGH - SPLASH_FIREWORK_GROW_WPM_LOW;
    uint32_t pos   = wpm - SPLASH_FIREWORK_GROW_WPM_LOW;
    uint32_t delta = (uint32_t)(SPLASH_MAX_RADIUS - SPLASH_FIREWORK_MAX_RADIUS) * pos / span;
    return (uint16_t)(SPLASH_FIREWORK_MAX_RADIUS + delta);
}

// トラックボール移動イベント
void rgblight_value(uint8_t row, uint8_t col, bool update, bool scr, bool splash_trig) {
    uint8_t last_index = led_index[last_row][last_col];

    // splash_trig(M-MODE/AZ1UBALLクリック時の明示的なスプラッシュ)を優先し、
    // それが無い場合のみ高速タイピング中(WPM>40)の花火風スプラッシュを登録する
    // (実際のキー座標更新イベント=update=trueに限る。update=falseは
    //  rgblight_effect_mousemove/scrollmoveからの毎フレームアニメーション更新
    //  呼び出しなので対象にしない。対象にすると毎フレーム発火し続けてしまう)。
    //
    // 花火風の最大到達半径は、発火した瞬間のWPMに応じて
    // splash_firework_radius_for_wpm()で決定し、そのスプラッシュが消えるまで固定する
    // (WPM40〜70: 小さい花火のまま固定。70〜100: 通常スプラッシュのサイズへ線形に拡大。
    //  100以上: 通常のクリック時スプラッシュと同じ大きさ)。
    if (splash_trig) {
        uint8_t slot                 = splash_alloc();
        splash_pool[slot].active     = true;
        splash_pool[slot].center_row = row;
        splash_pool[slot].center_col = col;
        splash_pool[slot].start_hue  = hue_value[last_index];
        splash_pool[slot].radius     = 0;
        splash_pool[slot].firework   = false;
        splash_pool[slot].max_radius = SPLASH_MAX_RADIUS;
    } else if (update && get_current_wpm() > 40) {
        uint8_t slot                 = splash_alloc();
        splash_pool[slot].active     = true;
        splash_pool[slot].center_row = row;
        splash_pool[slot].center_col = col;
        splash_pool[slot].start_hue  = hue_value[last_index];
        splash_pool[slot].radius     = 0;
        splash_pool[slot].firework   = true;
        splash_pool[slot].max_radius = splash_firework_radius_for_wpm(get_current_wpm());
    }

    // ── アクティブ全スプラッシュを処理（配列順 = 後勝ち上書き）──────
    for (uint8_t s = 0; s < SPLASH_MAX_COUNT; s++) {
        if (!splash_pool[s].active) continue;

        // 最大到達半径は発火時に決定済みの値をそのまま使う(毎フレーム再計算しない)
        uint16_t max_radius = splash_pool[s].max_radius;
        // firework(タイピング発火)の場合は花火風のWAVE_SOLID/WAVE_FADEを使う
        uint16_t wave_solid  = splash_pool[s].firework ? SPLASH_FIREWORK_WAVE_SOLID  : WAVE_SOLID;
        uint16_t wave_fade   = splash_pool[s].firework ? SPLASH_FIREWORK_WAVE_FADE   : WAVE_FADE;

        splash_pool[s].radius += SPLASH_SPEED;
        if (splash_pool[s].radius > max_radius) {
            splash_pool[s].active = false;
            continue;
        }

        uint16_t cur_radius = splash_pool[s].radius;

        for (uint8_t r = 0; r < MATRIX_ROWS; r++) {
            for (uint8_t c = 0; c < LED_MATRIX_COLS; c++) {
                uint8_t led_id = led_index[r][c];
                if (led_id == 59) continue;

                uint16_t d    = keyball_distance(r, c, splash_pool[s].center_row, splash_pool[s].center_col);
                int16_t  diff = (int16_t)d - (int16_t)cur_radius + SPLASH_LEAD_OFFSET;

                uint8_t v = rgblight_config.val;
                uint8_t h = splash_pool[s].start_hue;

                if (diff > DIST_SCALE) {
                    // 波面より外側：このスプラッシュの影響圏外なので書き込まない
                    continue;
                } else if (diff >= 0 && diff <= DIST_SCALE) {
                    v -= diff * SPLASH_TIP_FALLOFF;
                } else if (diff < 0 && diff >= -wave_solid) {
                    h -= diff >> 1;
                } else if (diff < -wave_solid && diff > -wave_fade) {
                    uint8_t x = (wave_fade - wave_solid) - (wave_fade - diff);
                    x = ((uint16_t)x * x) >> 4;
                    if (x > 150) x = 150;
                    v -= (150 - x);
                    h -= diff >> 1;
                } else {
                    // フェードアウト末尾（diff <= -wave_fade）：完全消灯域も書き込まない
                    continue;
                }
                // 影響圏内のみ後勝ちで上書き
                value[led_id]     = v;
                hue_value[led_id] = h;
            }
        }
    }

    // ── 通常のマウス移動フェード処理 ────────────────────────────────
    if (update) {
        if (last_row != row || last_col != col) {
            value[led_index[row][col]] = 255;
            if (scr) {
                for (uint8_t i = 0; i < 6; i++) {
                    value[led_index[row][i]] = 255;
                    hue_value[led_index[row][i]] = hue_value[last_index] - 20;
                }
            } else {
                hue_value[led_index[row][col]] = hue_value[last_index] - 16;
            }
        }
        last_row = row;
        last_col = col;
    } else {
        for (uint8_t i = 0; i < RGBLED_NUM; i++) value[i] = qsub8(value[i], 1);
        if (scr) {
            for (uint8_t i = 0; i < 6; i++) value[led_index[last_row][i]] = 255;
        } else {
            value[last_index] = 255;
        }
        for (uint8_t i = 0; i < RGBLED_NUM; i++) hue_value[i]--;
    }
}

// メインタスクループ
void rgblight_task(void) {

    if (rgblight_config.enable) {
        uint8_t interval_time = 255;
        switch (rgblight_config.mode) {
            case RGBLIGHT_MODE_SCROLLMOVE: interval_time = 8;  break;
            case RGBLIGHT_MODE_ICEWAVE:    interval_time = 16; break;
            case RGBLIGHT_MODE_MOUSEMOVE:  interval_time = 8;  break;
            case RGBLIGHT_MODE_STATIC:     interval_time = 50; break;
            case RGBLIGHT_MODE_SWIRL:      interval_time = 16; break;
            case RGBLIGHT_MODE_CROSS:      interval_time = 32; break;
        }

        if (timer_elapsed(effect_timer) >= interval_time) {
            effect_timer = timer_read();

            switch (rgblight_config.mode) {
                case RGBLIGHT_MODE_STATIC:     rgblight_effect_static();     break;
                case RGBLIGHT_MODE_ICEWAVE:    rgblight_effect_icewave();    break;
                case RGBLIGHT_MODE_MOUSEMOVE:  rgblight_effect_mousemove();  break;
                case RGBLIGHT_MODE_SCROLLMOVE: rgblight_effect_scrollmove(); break;
                case RGBLIGHT_MODE_SWIRL:      rgblight_effect_swirl();      break;
                case RGBLIGHT_MODE_CROSS:      rgblight_effect_cross();      break;
            }
        }
    }

    // 無操作が RGB_IDLE_TIMEOUT_MS 続いたら自動消灯する(旧 keymap.c の housekeeping_task_user)
    if (!rgb_is_idle && timer_elapsed32(last_rgb_activity) > RGB_IDLE_TIMEOUT_MS) {
        rgblight_disable();
        rgb_is_idle = true;
    }
}