#pragma once
#include <stdint.h>
#include "quantum.h"

// ジェスチャー系ステートをリセットする(レイヤー切り替え時やマウスモード切替時に呼ぶこと)
void process_gesture_layer_reset(void);

// 毎スキャン呼ぶこと。動きが無い時も x=0, y=0 で必ず呼ぶ(停止検知のため)
void process_gesture_layer(int16_t x, int16_t y, uint16_t up, uint16_t down, uint16_t left, uint16_t right);
void process_gesture_layer_func(int16_t h, int16_t v,
                            void (*on_up)(void), void (*on_down)(void),
                            void (*on_left)(void), void (*on_right)(void));