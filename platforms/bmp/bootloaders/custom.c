// Copyright 2023 sekigon-gonnoc
// SPDX-License-Identifier: GPL-2.0-or-later

#include "bootloader.h"

#include "apidef.h"

// TODO: bmp

__attribute__((weak)) void bootloader_jump(void) {
    BMPAPI->bootloader_jump();
}

__attribute__((weak)) void mcu_reset(void) {
    BMPAPI->app.reset(0);
}

__attribute__((weak)) void enter_bootloader_mode_if_requested(void) {}
