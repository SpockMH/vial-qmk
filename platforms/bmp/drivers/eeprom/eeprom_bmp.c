// Copyright 2023 sekigon-gonnoc
/* SPDX-License-Identifier: GPL-2.0-or-later */

#include <stdint.h>
#include <stddef.h>
#include <string.h>

#include "eeprom_driver.h"
#include "wait.h"
#include "print.h"
#include "eeconfig.h"
#include "via.h"
#include "version.h"

#include "eeprom_bmp.h"
#include "bmp_flash.h"
#include "apidef.h"

typedef struct {
    uint32_t magic;
    struct {
        uint16_t addr;
        uint16_t data;
    } log[(BMP_USER_FLASH_PAGE_SIZE - 4) / 4];

} flash_page_control_t;
_Static_assert(sizeof(flash_page_control_t) == BMP_USER_FLASH_PAGE_SIZE, "Invalid size");

typedef struct {
    union {
        uint8_t data[BMP_USER_FLASH_PAGE_SIZE * 2];
        struct {
            uint8_t page0[BMP_USER_FLASH_PAGE_SIZE];
            uint8_t page1[BMP_USER_FLASH_PAGE_SIZE];
        };
    };
} flash_page_data_t;
_Static_assert(sizeof(flash_page_data_t) == BMP_USER_FLASH_PAGE_SIZE * 2, "Invalid size");
_Static_assert(sizeof(flash_page_data_t) == EEPROM_SIZE, "Invalid size");

static flash_page_data_t     flash_data;
static flash_page_control_t  flash_control;
static uint32_t              log_write_idx;
static const uint32_t        control_log_max   = sizeof(flash_control.log) / sizeof(flash_control.log[0]);
static EEPROM_BMP_CACHE_MODE write_cache_mode  = EEPROM_BMP_CACHE_WRITE_BACK;
static bool                  write_cache_dirty = false;
const uint8_t               *eeprom_cache      = (uint8_t *)&flash_data;

static void truncate_flash_pages(void) {
    printf("Truncate flash pages\n");
    // Erase data
    flash_erase_page(FLASH_PAGE_ID_DATA0);
    flash_erase_page(FLASH_PAGE_ID_DATA1);
    // Write data
    flash_write_page(FLASH_PAGE_ID_DATA0, (uint32_t *)flash_data.page0);
    flash_write_page(FLASH_PAGE_ID_DATA1, (uint32_t *)flash_data.page1);

    if (flash_control.magic != BMP_FLASH_DRIVER_MAGIC || log_write_idx != 0) {
        // Erase write log
        flash_erase_page(FLASH_PAGE_ID_CONTROL);
        memset(&flash_control, 0xff, sizeof(flash_control));
        log_write_idx = 0;
        // Write magic number
        flash_control.magic = BMP_FLASH_DRIVER_MAGIC;
        flash_write_dword(FLASH_PAGE_ID_CONTROL * BMP_USER_FLASH_PAGE_SIZE, &flash_control.magic);
    }
}

void eeprom_driver_init(void) {
    flash_read(FLASH_PAGE_ID_CONTROL * BMP_USER_FLASH_PAGE_SIZE, (uint8_t *)&flash_control, BMP_USER_FLASH_PAGE_SIZE);
    log_write_idx = 0;

    if (flash_control.magic == BMP_FLASH_DRIVER_MAGIC) {
        flash_read(FLASH_PAGE_ID_DATA0 * BMP_USER_FLASH_PAGE_SIZE, (uint8_t *)&flash_data, sizeof(flash_data));

        // Apply write log
        for (int ofs = 0; ofs < control_log_max; ofs++) {
            uint16_t addr = flash_control.log[ofs].addr;
            uint16_t data = flash_control.log[ofs].data;
            if (addr > sizeof(flash_data) - 2) {
                break;
            } else {
                *(uint8_t *)(&flash_data.data[addr])     = data & 0xff;
                *(uint8_t *)(&flash_data.data[addr + 1]) = (data >> 8) & 0xff;
            }
            log_write_idx++;
        }
        // printf("read %ld log\n", log_write_idx);
    } else {
        // printf("magic %lx\n", flash_control.magic);
        if (write_cache_mode == EEPROM_BMP_CACHE_WRITE_THROUGH) {
            truncate_flash_pages();
        }
    }
}

void eeprom_driver_erase(void) {
    if (write_cache_mode == EEPROM_BMP_CACHE_WRITE_THROUGH) {
        truncate_flash_pages();
    }
    // flash_erase_page(FLASH_PAGE_ID_CONTROL);
    // flash_erase_page(FLASH_PAGE_ID_DATA);
}

void eeprom_read_block(void *buf, const void *addr, size_t len) {
    memcpy(buf, &flash_data.data[(uint32_t)addr], len);
}

static void bmp_flash_write_word(uint16_t data, uint32_t addr) {
    if (addr > sizeof(flash_data) - 2) {
        return;
    }

    if ((*(uint8_t *)(&flash_data.data[addr]) == (data & 0xff)) //
        && (*(uint8_t *)(&flash_data.data[addr + 1]) == ((data >> 8) & 0xff))) {
        // data is already written
        return;
    }

    if (log_write_idx >= control_log_max) {
        truncate_flash_pages();
    }

    *(uint8_t *)(&flash_data.data[addr])     = data & 0xff;
    *(uint8_t *)(&flash_data.data[addr + 1]) = (data >> 8) & 0xff;

    if (write_cache_mode == EEPROM_BMP_CACHE_WRITE_THROUGH) {
        if ((log_write_idx >= control_log_max)                  //
            || (flash_control.log[log_write_idx].addr != 0xffff //
                || flash_control.log[log_write_idx].data != 0xffff)) {
            // printf("Invalid write log found: %04lx, %04x, %04x\n", log_write_idx, //
            //        flash_control.log[log_write_idx].addr,                         //
            //        flash_control.log[log_write_idx].data);
            truncate_flash_pages();
        } else {
            flash_control.log[log_write_idx].addr = addr;
            flash_control.log[log_write_idx].data = data;
            // Write log
            flash_write_dword(offsetof(flash_page_control_t, log) + log_write_idx * sizeof(uint32_t), (uint32_t *)&flash_control.log[log_write_idx]);
            log_write_idx++;
        }
    } else {
        write_cache_dirty = true;
    }
}

static void bmp_flash_write_byte(uint8_t data, uint32_t addr) {
    uint32_t aligned_addr = addr & 0xfffffffe;
    uint16_t wdata        = 0;

    if (addr & 1) {
        wdata = (flash_data.data[aligned_addr]) | ((uint16_t)data << 8);
    } else {
        wdata = ((uint16_t)flash_data.data[aligned_addr + 1] << 8) | ((uint16_t)data);
    }
    bmp_flash_write_word(wdata, aligned_addr);
}

void eeprom_write_block(const void *buf, void *_addr, size_t len) {
    uint32_t       addr   = (uint32_t)_addr;
    const uint8_t *u8buff = (uint8_t *)buf;
    uint32_t       widx   = 0;
    if ((addr & 1) == 1) {
        bmp_flash_write_byte(u8buff[widx], addr);
        addr++;
        widx++;
        len--;
    }

    while (len > 0) {
        if (len >= 2) {
            uint16_t wd = u8buff[widx] | ((uint16_t)u8buff[widx + 1] << 8);
            bmp_flash_write_word(wd, addr);
            addr += 2;
            widx += 2;
            len -= 2;
        } else if (len == 1) {
            bmp_flash_write_byte(u8buff[widx], addr);
            addr++;
            widx++;
            len--;
        }
    }
}

void eeprom_bmp_flush(void) {
    if (write_cache_dirty) {
        truncate_flash_pages();
        write_cache_dirty = false;
    }
}

void eeprom_bmp_set_cache_mode(EEPROM_BMP_CACHE_MODE mode) {
    write_cache_mode = mode;
}

int eeprom_bmp_load_default(void) {
    uint16_t magic = 0;
    flash_read(FLASH_PAGE_ID_EEPROM_DEFAULT0 * BMP_USER_FLASH_PAGE_SIZE + (uint32_t)EECONFIG_MAGIC, (uint8_t *)&magic, 2);

    if (magic != EECONFIG_MAGIC_NUMBER) {
        return -1;
    }

    flash_read(FLASH_PAGE_ID_EEPROM_DEFAULT0 * BMP_USER_FLASH_PAGE_SIZE, (uint8_t *)&flash_data, sizeof(flash_data));

    flash_data.data[VIA_EEPROM_MAGIC_ADDR]     = BUILD_ID & 0xff;
    flash_data.data[VIA_EEPROM_MAGIC_ADDR + 1] = (BUILD_ID >> 8) & 0xff;
    flash_data.data[VIA_EEPROM_MAGIC_ADDR + 2] = (BUILD_ID >> 16) & 0xff;

    truncate_flash_pages();
    write_cache_dirty = false;

    return 0;
}

void eeprom_bmp_save_default(void) {
    // Erase data
    flash_erase_page(FLASH_PAGE_ID_EEPROM_DEFAULT0);
    flash_erase_page(FLASH_PAGE_ID_EEPROM_DEFAULT1);
    // Write data
    flash_write_page(FLASH_PAGE_ID_EEPROM_DEFAULT0, (uint32_t *)flash_data.page0);
    flash_write_page(FLASH_PAGE_ID_EEPROM_DEFAULT1, (uint32_t *)flash_data.page1);
}


void eeprom_bmp_erase_default(void) {
    // Erase data
    flash_erase_page(FLASH_PAGE_ID_EEPROM_DEFAULT0);
    flash_erase_page(FLASH_PAGE_ID_EEPROM_DEFAULT1);
}