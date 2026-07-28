#include "oled_7seg16.h"
#include <string.h>

// --- セグメントビットマップデータ (16x16px = 32 Bytes/seg) ---
const unsigned char seg7_16_A [] PROGMEM = {
	0x00, 0x00, 0x00, 0x01, 0x03, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x03, 0x01, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

const unsigned char seg7_16_B [] PROGMEM = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x3c, 0x7e, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

const unsigned char seg7_16_C [] PROGMEM = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0c, 0x1e, 0x3f, 0x00, 0x00
};

const unsigned char seg7_16_D [] PROGMEM = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x40, 0x60, 0x70, 0x70, 0x70, 0x70, 0x70, 0x70, 0x60, 0x40, 0x00, 0x00, 0x00
};

const unsigned char seg7_16_E [] PROGMEM = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x3f, 0x1e, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

const unsigned char seg7_16_F [] PROGMEM = {
	0x00, 0x00, 0x7e, 0x3c, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

const unsigned char seg7_16_G [] PROGMEM = {
	0x00, 0x00, 0x00, 0x80, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0x80, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00
};


const unsigned char* const seg7_16_allArray[7] PROGMEM = {
    seg7_16_A, seg7_16_B, seg7_16_C, seg7_16_D, seg7_16_E, seg7_16_F, seg7_16_G
};

// セグメント定義
#define SEG16_A (1 << 0)
#define SEG16_B (1 << 1)
#define SEG16_C (1 << 2)
#define SEG16_D (1 << 3)
#define SEG16_E (1 << 4)
#define SEG16_F (1 << 5)
#define SEG16_G (1 << 6)

static uint8_t get_seg_mask(char c) {
    switch (c) {
        case '0': return SEG16_A | SEG16_B | SEG16_C | SEG16_D | SEG16_E | SEG16_F;
        case '1': return SEG16_B | SEG16_C;
        case '2': return SEG16_A | SEG16_B | SEG16_G | SEG16_E | SEG16_D;
        case '3': return SEG16_A | SEG16_B | SEG16_G | SEG16_C | SEG16_D;
        case '4': return SEG16_F | SEG16_G | SEG16_B | SEG16_C;
        case '5': return SEG16_A | SEG16_F | SEG16_G | SEG16_C | SEG16_D;
        case '6': return SEG16_A | SEG16_F | SEG16_E | SEG16_D | SEG16_C | SEG16_G;
        case '7': return SEG16_A | SEG16_B | SEG16_C;
        case '8': return SEG16_A | SEG16_B | SEG16_C | SEG16_D | SEG16_E | SEG16_F | SEG16_G;
        case '9': return SEG16_A | SEG16_B | SEG16_C | SEG16_D | SEG16_F | SEG16_G;
        case 'A':
        case 'a': return SEG16_A | SEG16_B | SEG16_C | SEG16_E | SEG16_F | SEG16_G;
        case 'B':
        case 'b': return SEG16_F | SEG16_E | SEG16_G | SEG16_C | SEG16_D;
        case 'C':
        case 'c': return SEG16_A | SEG16_F | SEG16_E | SEG16_D;
        case 'D':
        case 'd': return SEG16_B | SEG16_G | SEG16_E | SEG16_D | SEG16_C;
        case 'E':
        case 'e': return SEG16_A | SEG16_F | SEG16_G | SEG16_E | SEG16_D;
        case 'F':
        case 'f': return SEG16_A | SEG16_F | SEG16_G | SEG16_E;
        case '-': return SEG16_G;
        default:  return 0x00;
    }
}

void oled_write_7seg16_char(uint8_t x, uint8_t y, char c) {
    uint8_t mask = get_seg_mask(c);
    char buffer[OLED_7SEG16_BYTES];
    memset(buffer, 0, sizeof(buffer));

    // マスクに従って各セグメントデータをOR合成
    for (uint8_t i = 0; i < 7; i++) {
        if (mask & (1 << i)) {
            const unsigned char* seg_ptr = (const unsigned char*)pgm_read_ptr(&seg7_16_allArray[i]);
            for (uint16_t b = 0; b < OLED_7SEG16_BYTES; b++) {
                buffer[b] |= pgm_read_byte(&seg_ptr[b]);
            }
        }
    }

    // OLED画面の指定位置へ書き込み
    for (uint8_t page = 0; page < 2; page++) {
        oled_set_cursor(x / 6, (y / 8) + page);
        oled_write_raw(&buffer[page * OLED_7SEG16_WIDTH], OLED_7SEG16_WIDTH);
    }
}

void oled_write_7seg16_num(uint8_t x, uint8_t y, uint8_t num) {
    if (num > 9) {
        num = 9;
    }
    oled_write_7seg16_char(x, y, '0' + num);
}

void oled_write_7seg16_num2(uint8_t x, uint8_t y, uint8_t num) {
    // 0～99に制限
    if (num > 99) {
        num = 99;
    }

    // 十の位と一の位を取得
    uint8_t tens = num / 10;
    uint8_t ones = num % 10;

    // 32×16ピクセル
    // 1ページ = 横32バイト
    // 16ピクセル = 2ページ
    char buffer[64];

    memset(buffer, 0, sizeof(buffer));

    // 各数字のセグメントマスクを取得
    uint8_t tens_mask = get_seg_mask('0' + tens);
    uint8_t ones_mask = get_seg_mask('0' + ones);

    // 7セグメントを合成
    for (uint8_t i = 0; i < 7; i++) {

        const uint8_t *seg_ptr =
            (const uint8_t *)pgm_read_ptr(
                &seg7_16_allArray[i]
            );

        // 十の位を左側へ合成
        if (tens_mask & (1 << i)) {
            for (uint8_t b = 0; b < OLED_7SEG16_BYTES; b++) {

                uint8_t page = b / OLED_7SEG16_WIDTH;
                uint8_t col  = b % OLED_7SEG16_WIDTH;

                // 左側：横0～15
                buffer[
                    page * 32 + col
                ] |= pgm_read_byte(&seg_ptr[b]);
            }
        }

        // 一の位を右側へ合成
        if (ones_mask & (1 << i)) {
            for (uint8_t b = 0; b < OLED_7SEG16_BYTES; b++) {

                uint8_t page = b / OLED_7SEG16_WIDTH;
                uint8_t col  = b % OLED_7SEG16_WIDTH;

                // 右側：横16～31
                buffer[
                    page * 32 + 16 + col
                ] |= pgm_read_byte(&seg_ptr[b]);
            }
        }
    }

    // 32×16を一度にOLEDへ書き込む
    for (uint8_t page = 0; page < 2; page++) {

        oled_set_cursor(x / 6, (y / 8) + page);

        oled_write_raw(&buffer[page * 32], 32);
    }
}