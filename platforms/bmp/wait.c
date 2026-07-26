// Copyright 2023 sekigon-gonnoc
/* SPDX-License-Identifier: GPL-2.0-or-later */

#include "_wait.h"

// BMP
#include "apidef.h"

// TODO: bmp

void wait_us(uint16_t duration) {
    BMPAPI->timer.sleep_us(duration);
}

void wait_ms(uint16_t duration) {
    BMPAPI->timer.sleep_ms(duration);
}