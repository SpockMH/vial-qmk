#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "quantum.h"

// keymap.c側の独自キーコードenumで実際の値を割り当て、これらの変数に代入すること。
// (select_word_lr.h と同じ「weakなextern変数に外から代入する」パターン)
extern uint16_t SPD_THL_UP_KEYCODE; // 下限閾値を上げる
extern uint16_t SPD_THL_DN_KEYCODE; // 下限閾値を下げる
extern uint16_t SPD_THU_UP_KEYCODE; // 上限閾値を上げる
extern uint16_t SPD_THU_DN_KEYCODE; // 上限閾値を下げる
extern uint16_t SPD_SCL_UP_KEYCODE; // 下限側の最小倍率を上げる
extern uint16_t SPD_SCL_DN_KEYCODE; // 下限側の最小倍率を下げる

// 直近の移動量の大きさ(|x|+|y|)をリングバッファに追加する。
// keyball_on_apply_motion_to_mouse_move() 内、PMW3360側の時だけ呼ぶこと。
void mouse_speed_smoothing_push(int16_t magnitude);

// リングバッファの現在の合計値をもとに、vに速度スムージング倍率を適用して返す。
int16_t mouse_speed_smoothing_apply(int16_t v);

// 6個の調整キーを処理する。process_record_userから呼ぶこと。
bool process_mouse_speed_smoothing(uint16_t keycode, keyrecord_t *record);

// ---- OLED表示用のgetter ----
int16_t mouse_speed_smoothing_get_lower_threshold(void);
int16_t mouse_speed_smoothing_get_upper_threshold(void);
uint8_t mouse_speed_smoothing_get_min_scale_pct(void);
int32_t mouse_speed_smoothing_get_current_sum(void);       // 直近のリングバッファ合計値(体感確認用)
uint8_t mouse_speed_smoothing_get_current_scale_pct(void); // 直近適用された倍率(%)

// ---- EEPROM永続化用(eeconfig_user.cから呼ぶ) ----
void mouse_speed_smoothing_set_config(int16_t lower_threshold, int16_t upper_threshold, uint8_t min_scale_pct);
void mouse_speed_smoothing_get_config(int16_t *lower_threshold, int16_t *upper_threshold, uint8_t *min_scale_pct);