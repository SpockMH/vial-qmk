// Copyright 2023 sekigon-gonnoc
/* SPDX-License-Identifier: GPL-2.0-or-later */

#include "timer.h"

// BMP
#include "apidef.h"

// TODO: bmp

void timer_init(void) {}

void timer_clear(void) {}

uint16_t timer_read(void) {
    return BMPAPI->timer.get_ms() & 0xffff;
}

uint32_t timer_read32(void) {
    // 27bit
    return BMPAPI->timer.get_ms();
}

uint16_t timer_elapsed(uint16_t last) {
    return TIMER_DIFF_16(timer_read(), last);
}

uint32_t timer_elapsed32(uint32_t last) {
    return TIMER_DIFF(timer_read32(), last, (1 << 27));
}