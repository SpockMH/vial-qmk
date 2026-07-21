#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "quantum.h"
 
// 十字キー4方向用の仮想マトリクス位置(info.json上の未使用スロット)。
// Vialのキーマップ編集画面から、この4箇所に自由にキーを割り当てられる。
#define AZ_DPAD_UP_ROW    3
#define AZ_DPAD_UP_COL    0
#define AZ_DPAD_DOWN_ROW  7
#define AZ_DPAD_DOWN_COL  0
#define AZ_DPAD_LEFT_ROW  7
#define AZ_DPAD_LEFT_COL  2
#define AZ_DPAD_RIGHT_ROW 7
#define AZ_DPAD_RIGHT_COL 3

// ジェスチャー系ステートをリセットする(レイヤー切り替え時やマウスモード切替時に呼ぶこと)
void process_gesture_layer_reset(void);

// 毎スキャン呼ぶこと。動きが無い時も x=0, y=0 で必ず呼ぶ(停止検知のため)
void process_gesture_layer(int16_t x, int16_t y, uint16_t up, uint16_t down, uint16_t left, uint16_t right);
void process_gesture_layer_func(int16_t h, int16_t v,
                            void (*on_up)(void), void (*on_down)(void),
                            void (*on_left)(void), void (*on_right)(void));
void process_gesture_layer_key(int16_t h, int16_t v);