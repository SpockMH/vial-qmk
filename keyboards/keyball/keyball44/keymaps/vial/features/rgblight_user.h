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
    RGBLIGHT_MODE_SCROLLMOVE
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

extern rgblight_simple_config_t rgblight_config;

/* API Functions */
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

// 【復活】子機側でデータ同期を受信した際に呼び出す関数
void rgblight_update_sync(rgblight_simple_config_t *master_config);
// トラックボール通知用
void rgblight_value(uint8_t row, uint8_t col, bool update, bool scr, bool splash_trig);

uint8_t get_hue(void);
void set_hue(uint8_t value);
