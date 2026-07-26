// Copyright 2023 sekigon-gonnoc
/* SPDX-License-Identifier: GPL-2.0-or-later */

#pragma once

#include <stdint.h>

#include "apidef.h"

#define BMP_VIAL_FLASH_PAGE_MAGIC 0xB05AFAAE
#define VIAL_SUPPORT_VIA_VERSION 0x0009

typedef struct {
    uint32_t         magic;
    uint32_t         len;
    uint8_t          vial_data[BMP_USER_FLASH_PAGE_SIZE - 2 * sizeof(uint32_t) - 8 - sizeof(bmp_api_config_t) - 64 - sizeof(uint32_t)];
    uint8_t          vial_uid[8];
    bmp_api_config_t bmp_config;
    uint8_t          reserved[64];
    uint32_t         crc16;       
} flash_vial_data_t;

extern flash_vial_data_t flash_vial_data;

const flash_vial_data_t *bmp_vial_get_config(void);
void                     bmp_vial_data_init(void);
void                     bmp_raw_hid_receive(const uint8_t *data, uint8_t len);
void                     bmp_vial_save_config(void);
void                     bmp_set_vial_enable_flag(bool flag);
void                     bmp_raw_hid_receive_common(const uint8_t *data, uint8_t len);