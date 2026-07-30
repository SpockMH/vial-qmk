#pragma once

#include "quantum.h"
#include "oled_driver.h"

// 1桁分の7segデータの寸法
#define OLED_SEG7_16_WIDTH   11  // 1桁あたりの幅(左右1pxずつ余白込み)
#define OLED_SEG7_16_HEIGHT  16
#define OLED_SEG7_16_PAGES   (OLED_SEG7_16_HEIGHT / 8) // = 2
#define OLED_SEG7_16_BYTES   (OLED_SEG7_16_WIDTH * OLED_SEG7_16_PAGES) // = 22

// 3桁まとめて描画したときの合計幅
// 11px×3桁=33pxのうち、末尾の不要な1px(3桁目の右余白)を切り捨てて32pxにする
#define OLED_SEG7_16_3DIGIT_WIDTH (OLED_SEG7_16_WIDTH * 3 - 1) // = 32

/**
 * 指定座標(x, y)に、7セグメント風の1文字を描画します(11x16px)。
 *
 * @param x 表示位置のX座標
 * @param y 表示位置のY座標
 * @param c 描画する文字 ('0'〜'9', '-', ' ')
 */
void oled_write_7seg16_char(uint8_t x, uint8_t y, char c);

/**
 * 指定座標(x, y)に、1桁の数値(0〜9)を描画します(11x16px)。
 */
void oled_write_7seg16_num(uint8_t x, uint8_t y, uint8_t num);

/**
 * 指定座標(x, y)に、3桁の数値(0〜999)をまとめて描画します(32x16px)。
 * 100の位が0の場合も3桁固定でゼロ埋め表示します。
 */
void oled_write_7seg16_num3(uint8_t x, uint8_t y, uint16_t num);