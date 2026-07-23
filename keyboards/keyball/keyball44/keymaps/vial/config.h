/*
This is the c configuration file for the keymap

Copyright 2022 @Yowkees
Copyright 2022 MURAOKA Taro (aka KoRoN, @kaoriya)

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hopekt even the implied warranty of
MERCHANTABILITjY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

// vial-qmk/platforms/chibios/vendors/RP/RP2040.mk
#pragma once
// Vial 用: キーボード固有のUID（8バイト）
#define VIAL_KEYBOARD_UID {0xF5, 0x86, 0x83, 0x91, 0x23, 0x12, 0x17, 0x5F}
#define VIAL_UNLOCK_COMBO_ROWS {0, 0}
#define VIAL_UNLOCK_COMBO_COLS {0, 1}


#define AZ1UBALL_ENABLE
#undef LAYER_STATE_8BIT
#define LAYER_STATE_16BIT
#define DYNAMIC_KEYMAP_LAYER_COUNT 8
#define VIALRGB_NO_DIRECT

#define USB_MAX_POWER_CONSUMPTION 500

#define TAP_CODE_DELAY 0
#define COMBO_TERM 40
#define KEYBALL_CPI_DEFAULT 700      // 光学センサーPMW3360DM の解像度 (CPI) の規定値
#define KEYBALL_SCROLL_DIV_DEFAULT 4
#define KEYBALL_SCROLLSNAP_ENABLE 1

#define SPLIT_TRANSACTION_IDS_USER USER_SYNC_KEY_COUNTER, USER_SYNC_LIGHTING

#define WS2812_DRIVER_REQUIRED
#define WS2812_DI_PIN       GP0
#define RGBLED_NUM          60
#define WS2812_LED_COUNT    RGBLED_NUM
#define LEFT_OFFSET         30


#define OLED_FONT_H "features/font.c"
#define OLED_FONT_START 48
#define OLED_FONT_END 90

// #define SPLIT_WPM_ENABLE
// #define SPLIT_LED_STATE_ENABLE
// #define SPLIT_LAYER_STATE_ENABLE
//#define SPLIT_TRANSPORT_MIRROR

#undef DEBOUNCE
#define DEBOUNCE 5
#define USB_POLLING_INTERVAL_MS 5
#define SELECT_SOFT_SERIAL_SPEED 1
#define EECONFIG_USER_DATA_SIZE 16

// --- ハイレゾ・拡張レポートの設定 ---
#define WHEEL_EXTENDED_REPORT
#define POINTING_DEVICE_HIRES_SCROLL_ENABLE
#define POINTING_DEVICE_HIRES_SCROLL_MULTIPLIER 120

// --- マウスキーの設定（120から1に修正） ---
#define MOUSEKEY_WHEEL_DELTA 1