/* SPDX-License-Identifier: GPL-2.0-or-later */

// System
#include <string.h>

// QMK
#include "spi_master.h"

// BMP
#include "apidef.h"

#ifndef CONFIG_SCK_PIN
#define CONFIG_SCK_PIN 16
#endif

#ifndef CONFIG_MISO_PIN
#define CONFIG_MISO_PIN 15
#endif

#ifndef CONFIG_MOSI_PIN
#define CONFIG_MOSI_PIN 14
#endif

#ifndef CONFIG_SS_PIN
#define CONFIG_SS_PIN 0
#endif

#ifndef CONFIG_HALF_DUPLEX_PIN
#define CONFIG_HALF_DUPLEX_PIN 0
#endif

#ifndef CONFIG_SPI_FREQ
#define CONFIG_SPI_FREQ SPI_FREQ_1M
#endif

#ifndef CONFIG_SPI_MODE
#define CONFIG_SPI_MODE 3
#endif

static pin_t currentSlave;

void spi_init(void) {
    bmp_api_spim_config_t config = {
        .freq            = CONFIG_SPI_FREQ,
        .miso            = CONFIG_MISO_PIN,
        .mosi            = CONFIG_MOSI_PIN,
        .sck             = CONFIG_SCK_PIN,
        .mode            = CONFIG_SPI_MODE,
        .half_duplex_pin = CONFIG_HALF_DUPLEX_PIN,
    };
    BMPAPI->spim.init(&config);
}

bool spi_start(pin_t slavePin, bool lsbFirst, uint8_t mode, uint16_t divisor) {
    gpio_set_pin_output(slavePin);
    gpio_write_pin_low(slavePin);
    currentSlave = slavePin;

    return true;
}

spi_status_t spi_write(uint8_t data) {
    uint8_t ret=0;
    BMPAPI->spim.start(&data, 1, &ret, 1, 0xFF);
    return ret;
}

spi_status_t spi_read(void) {
    uint8_t dummy = 0;
    uint8_t ret   = 0;
    BMPAPI->spim.start(&dummy, 1, &ret, 1, 0xFF);
    return ret;
}

spi_status_t spi_transmit(const uint8_t *data, uint16_t length) {
    uint8_t data_ram[length];
    memcpy(data_ram, data, length);
    BMPAPI->spim.start(data_ram, length, NULL, 0, 0xFF);

    return SPI_STATUS_SUCCESS;
}

spi_status_t spi_receive(uint8_t *data, uint16_t length) {
    BMPAPI->spim.start(NULL, 0, data, length, 0xFF);

    return SPI_STATUS_SUCCESS;
}

void spi_stop(void) {
    gpio_set_pin_output(currentSlave);
    gpio_write_pin_high(currentSlave);
}