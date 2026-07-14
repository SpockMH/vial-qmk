#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "quantum.h"

// AZ1UBALL I2C アドレス (固定値。変更不可)
#define AZ1UBALL_I2C_ADDR 0x0A

// I2C タイムアウト [ms]
#define AZ1UBALL_I2C_TIMEOUT 5

// カウントモード: 0=通常(常に1加算), 1=AZ用(速度に応じて1/2/3加算)
#ifndef AZ1UBALL_COUNT_MODE
#    define AZ1UBALL_COUNT_MODE 1
#endif

// 感度スケーリング (移動量にかける倍率)
#ifndef AZ1UBALL_SCALE
#    define AZ1UBALL_SCALE 1
#endif

/**
 * @brief AZ1UBALL を初期化する。
 *        keyboard_post_init_user() の中から呼ぶこと。
 * @return true  初期化成功
 * @return false デバイス未検出
 */
bool az1uball_init(void);

/**
 * @brief モーションデータを取得する。
 * @param[out] x      X 軸移動量 (右が正)
 * @param[out] y      Y 軸移動量 (下が正)
 * @param[out] btn    ボタン押下中なら true
 * @return true  新しいデータあり (x/y/btn のいずれかが非ゼロ)
 * @return false 動きなし / エラー
 */
bool az1uball_get_motion(int16_t *x, int16_t *y, bool *btn);

/**
 * @brief report_mouse_t形式でモーションを取得する。
 *        pointing_device_driver_get_report()から呼ぶことを想定。
 *        x/yに動き、buttonsのbit0にクリック状態を格納する。
 *        戻り値のreport_mouse_tはQMKコアがそのままsplit通信で運ぶ。
 */
report_mouse_t az1uball_get_report(report_mouse_t mouse_report);