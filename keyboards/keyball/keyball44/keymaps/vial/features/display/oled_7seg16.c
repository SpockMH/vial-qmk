#include "oled_7seg16.h"
#include <string.h>

// --- 11x16pxセグメントビットマップ(ページ単位: [0]=行0-7, [1]=行8-15) ---
const unsigned char seg7_16_A[] PROGMEM = {
    0x00, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
const unsigned char seg7_16_B[] PROGMEM = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x00
};
const unsigned char seg7_16_C[] PROGMEM = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0xc0, 0xc0, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7f, 0x7f, 0x7f, 0x00
};
const unsigned char seg7_16_D[] PROGMEM = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x70, 0x70, 0x70, 0x70, 0x70, 0x70, 0x70, 0x70, 0x70, 0x00
};
const unsigned char seg7_16_E[] PROGMEM = {
    0x00, 0xc0, 0xc0, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x7f, 0x7f, 0x7f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
const unsigned char seg7_16_F[] PROGMEM = {
    0x00, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
const unsigned char seg7_16_G[] PROGMEM = {
    0x00, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0x00,
    0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00
};

const unsigned char* const seg7_16_allArray[7] PROGMEM = {
    seg7_16_A, seg7_16_B, seg7_16_C, seg7_16_D, seg7_16_E, seg7_16_F, seg7_16_G
};

#define SEG_A (1 << 0)
#define SEG_B (1 << 1)
#define SEG_C (1 << 2)
#define SEG_D (1 << 3)
#define SEG_E (1 << 4)
#define SEG_F (1 << 5)
#define SEG_G (1 << 6)

static uint8_t get_seg_mask(char c) {
    switch (c) {
        case '0': return SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F;
        case '1': return SEG_B | SEG_C;
        case '2': return SEG_A | SEG_B | SEG_G | SEG_E | SEG_D;
        case '3': return SEG_A | SEG_B | SEG_G | SEG_C | SEG_D;
        case '4': return SEG_F | SEG_G | SEG_B | SEG_C;
        case '5': return SEG_A | SEG_F | SEG_G | SEG_C | SEG_D;
        case '6': return SEG_A | SEG_F | SEG_E | SEG_D | SEG_C | SEG_G;
        case '7': return SEG_A | SEG_B | SEG_C;
        case '8': return SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F | SEG_G;
        case '9': return SEG_A | SEG_B | SEG_C | SEG_D | SEG_F | SEG_G;
        case '-': return SEG_G;
        default:  return 0x00;
    }
}
// 1桁分(11x16px)のビットマップをOR合成してbufferへ書き込む
static void compose_digit(char c, char *buffer) {
    uint8_t mask = get_seg_mask(c);
    memset(buffer, 0, OLED_SEG7_16_BYTES);

    for (uint8_t i = 0; i < 7; i++) {
        if (mask & (1 << i)) {
            const unsigned char* seg_ptr = (const unsigned char*)pgm_read_ptr(&seg7_16_allArray[i]);
            for (uint16_t b = 0; b < OLED_SEG7_16_BYTES; b++) {
                buffer[b] |= pgm_read_byte(&seg_ptr[b]);
            }
        }
    }
}

void oled_write_7seg16_char(uint8_t x, uint8_t y, char c) {
    char buffer[OLED_SEG7_16_BYTES];
    compose_digit(c, buffer);

    for (uint8_t page = 0; page < OLED_SEG7_16_PAGES; page++) {
        oled_set_cursor(x / 6, (y / 8) + page);
        oled_write_raw(&buffer[page * OLED_SEG7_16_WIDTH], OLED_SEG7_16_WIDTH);
    }
}

void oled_write_7seg16_num(uint8_t x, uint8_t y, uint8_t num) {
    if (num > 9) {
        num = 9;
    }
    oled_write_7seg16_char(x, y, '0' + num);
}

void oled_write_7seg16_num3(uint8_t x, uint8_t y, uint16_t num) {
    if (num > 999) {
        num = 999;
    }

    char digits[3] = {
        (char)('0' + (num / 100) % 10),
        (char)('0' + (num / 10)  % 10),
        (char)('0' + num % 10),
    };

    char combined[OLED_SEG7_16_3DIGIT_WIDTH * OLED_SEG7_16_PAGES];
    memset(combined, 0, sizeof(combined));

    char digit_buf[OLED_SEG7_16_BYTES];

    for (uint8_t d = 0; d < 3; d++) {
        compose_digit(digits[d], digit_buf);

        uint8_t x_offset = d * OLED_SEG7_16_WIDTH; // 0, 11, 22

        for (uint8_t page = 0; page < OLED_SEG7_16_PAGES; page++) {
            for (uint8_t col = 0; col < OLED_SEG7_16_WIDTH; col++) {
                uint8_t gx = x_offset + col;
                if (gx >= OLED_SEG7_16_3DIGIT_WIDTH) {
                    continue; // 33px目(3桁目の右余白)は切り捨て
                }
                combined[page * OLED_SEG7_16_3DIGIT_WIDTH + gx] |=
                    digit_buf[page * OLED_SEG7_16_WIDTH + col];
            }
        }
    }

    for (uint8_t page = 0; page < OLED_SEG7_16_PAGES; page++) {
        oled_set_cursor(x / 6, (y / 8) + page);
        oled_write_raw(&combined[page * OLED_SEG7_16_3DIGIT_WIDTH], OLED_SEG7_16_3DIGIT_WIDTH);
    }
}