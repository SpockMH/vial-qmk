// Copyright 2023 sekigon-gonnoc
/* SPDX-License-Identifier: GPL-2.0-or-later */

#include "wait.h"

#include "bmp_flash.h"
#include "apidef.h"

void flash_write_dword(uint32_t addr, uint32_t *data) {
    int retry = 3;
    int res   = 0;
    while (1) {
        res = BMPAPI->flash.write_dword(addr, data);
        if (res == 0 || --retry == 0) {
            break;
        }
        wait_us(10000); // Do NOT call wait_ms here because wait_ms calls usb tasks recursively
        BMPAPI->app.process_task();
    }
}

void flash_write_page(uint32_t page, uint32_t *data) {
    int retry = 3;
    int res   = 0;
    while (1) {
        res = BMPAPI->flash.write_page(page, data);
        if (res == 0 || --retry == 0) {
            break;
        }
        wait_us(10000);
        BMPAPI->app.process_task();
    }
}

void flash_read(uint32_t addr, uint8_t *data, uint32_t len) {
    BMPAPI->flash.read(addr, data, len);
}

int flash_erase_page(uint32_t page) {
    int retry = 3;
    int res   = 0;
    while (1) {
        res = BMPAPI->flash.erase_page(page);
        if (res == 0 || --retry == 0) {
            return 0;
        }
        wait_us(10000);
        BMPAPI->app.process_task();
    }

    return -1;
}