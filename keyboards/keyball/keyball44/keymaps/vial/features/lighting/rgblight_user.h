/* rgblight_user.h */
#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "ws2812.h"

// モード定義
enum RGBLIGHT_EFFECT_MODE {
    RGBLIGHT_MODE_OFF = 0,
    RGBLIGHT_MODE_ICEWAVE,
    RGBLIGHT_MODE_STATIC,
    RGBLIGHT_MODE_MOUSEMOVE,
    RGBLIGHT_MODE_SCROLLMOVE,
    RGBLIGHT_MODE_SWIRL,
    RGBLIGHT_MODE_CROSS,
};

// 簡易的な設定保持構造体
typedef struct {
    bool    enable;
    uint8_t mode;
    uint8_t hue;
    uint8_t sat;
    uint8_t val;
} rgblight_simple_config_t;


// 同期フラグの定義
#define RGBLIGHT_STATUS_CHANGE_MODE (1 << 0)
#define RGBLIGHT_STATUS_CHANGE_HSVS (1 << 1)

extern rgblight_simple_config_t rgblikght_config;

/* API Functions */
// rgblight_init は内部で lighting_register_rpc_handlers() の呼び出しと
// アイドル管理状klll-lj態の初期化もまとめて行う。keyboard_post_init_user からは
// これ1つを呼べばよい。
void rgblight_init(void);
void rgblight_task(void);
void rgblight_mode(uint8_t mode);
void rgblight_sethsv(uint8_t hue, uint8_t sat, uint8_t val);

// 状態取得・変更関数
bool rgblight_is_enabled(void);

uint8_t rgblight_get_val(void);
void rgblight_toggle(void);
void rgblight_enable(void);
void rgblight_disable(void);
void rgblight_increase_val(void);
void rgblight_decrease_val(void);

// キー入力等の操作があった際に呼ぶ。アイドル消灯中であれば再点灯する。
// process_record_user から呼ぶこと。
void rgblight_wake(void);

// 【復活】子機側でデータ同期を受信した際に呼び出す関数
void rgblight_update_sync(rgblight_simple_config_t *master_config);

// マスターからの USER_SYNC_LIGHTING RPC を受け取り rgblight_update_sync() に渡す。
void rgblight_sync_rpc_handler(uint8_t in_buflen, const void *in_data, uint8_t out_buflen, void *out_data);

// ライティング関連のRPCハンドラ(USER_SYNC_KEY_COUNTER / USER_SYNC_LIGHTING)を
// まとめて登録する。rgblight_init() から呼ばれる(直接呼ぶ必要はない)。
void lighting_register_rpc_handlers(void);

// トラックボール通知用
void rgblight_value(uint8_t row, uint8_t col, bool update, bool scr, bool splash_trig);

uint8_t get_hue(void);
void set_hue(uint8_t value);
