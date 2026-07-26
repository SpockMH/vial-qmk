#pragma once

#include "quantum.h"
#include "oled_driver.h"

#define OLED_SEG7_WIDTH  32
#define OLED_SEG7_HEIGHT 32
#define OLED_SEG7_BYTES  128 // 32 * (32 / 8)

/**
 * 指定した座標(x, y)に、7セグメント風の文字を描画します。
 * 
 * @param x 表示位置のX座標 (0 ～ OLED_WIDTH - 32)
 * @param y 表示位置のY座標 (0 ～ OLED_HEIGHT - 32)
 * @param c 描画する文字 ('0'～'9', 'A', 'b', 'C', 'd', 'E', 'F', ' ', '-')
 */
void oled_write_7seg_char(uint8_t x, uint8_t y, char c);

/**
 * 指定した座標(x, y)に、1桁の数値(0～9)を描画します。
 * 
 * @param x 表示位置のX座標
 * @param y 表示位置のY座標
 * @param num 表示する数値 (0～9)
 */
void oled_write_7seg_num(uint8_t x, uint8_t y, uint8_t num);