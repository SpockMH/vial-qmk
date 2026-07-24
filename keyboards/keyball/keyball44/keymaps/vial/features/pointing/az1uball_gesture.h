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
#define AZ_DPAD_CLICK_ROW 3
#define AZ_DPAD_CLICK_COL 6

void process_az1uball_gesture(int16_t h, int16_t v, bool click);