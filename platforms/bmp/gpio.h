/*
Copyright 2021 Sekigon

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include "apidef.h"

typedef uint8_t                  pin_t;
static const bmp_api_gpio_mode_t bmp_gpio_in_pu = {
    .dir  = BMP_MODE_INPUT,
    .pull = BMP_PULLUP,
};
static const bmp_api_gpio_mode_t bmp_gpio_in_pd = {
    .dir  = BMP_MODE_INPUT,
    .pull = BMP_PULLDOWN,
};
static const bmp_api_gpio_mode_t bmp_gpio_default = {
    .dir  = BMP_MODE_DEFAULT,
};
static const bmp_api_gpio_mode_t bmp_gpio_out_od = {
    .dir = BMP_MODE_OUTPUT,
    .pull = BMP_PULL_NONE,
    .drive = BMP_PIN_S0D1
};
static const bmp_api_gpio_mode_t bmp_gpio_inout  = {.dir = BMP_MODE_INOUT, .pull = BMP_PULL_NONE, .drive = BMP_PIN_S0S1};
static const bmp_api_gpio_mode_t bmp_gpio_out_pp = {.dir = BMP_MODE_OUTPUT, .pull = BMP_PULL_NONE, .drive = BMP_PIN_S0S1};
#define gpio_set_pin_input(pin) BMPAPI->gpio.set_mode(pin, &bmp_gpio_in_pu)
#define gpio_set_pin_input_high(pin) BMPAPI->gpio.set_mode(pin, &bmp_gpio_in_pu)
#define gpio_set_pin_input_low(pin) BMPAPI->gpio.set_mode(pin, &bmp_gpio_in_pd)
#define gpio_set_pin_output_push_pull(pin) BMPAPI->gpio.set_mode(pin, &bmp_gpio_out_pp)
#define gpio_set_pin_output_open_drain(pin) BMPAPI->gpio.set_mode(pin, &bmp_gpio_out_od)
#define gpio_set_pin_output(pin) gpio_set_pin_output_push_pull(pin)
#define gpio_write_pin_high(pin) BMPAPI->gpio.set_pin(pin)
#define gpio_write_pin_low(pin) BMPAPI->gpio.clear_pin(pin)
#define gpio_write_pin(pin, level) ((level) ? writePinHigh(pin) : writePinLow(pin))

#define gpio_read_pin(pin) BMPAPI->gpio.read_pin(pin)

#define setPinInOut(pin) BMPAPI->gpio.set_mode(pin, &bmp_gpio_inout)
#define setPinDefault(pin) BMPAPI->gpio.set_mode(pin, &bmp_gpio_default)