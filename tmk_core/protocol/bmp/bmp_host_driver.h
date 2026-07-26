// Copyright 2023 sekigon-gonnoc
/* SPDX-License-Identifier: GPL-2.0-or-later */
#pragma once

#include <stdbool.h>

bool get_ble_enabled(void);
void set_ble_enabled(bool enabled);
bool get_usb_enabled(void);
void set_usb_enabled(bool enabled);
void select_ble(void);
void select_usb(void);
bool is_ble_connected(void);
bool is_usb_connected(void);
void bmp_raw_hid_receive_usb(const uint8_t *data, uint8_t len);
void bmp_raw_hid_receive_ble(const uint8_t *data, uint8_t len);
