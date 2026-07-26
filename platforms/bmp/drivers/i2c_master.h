
#pragma once
#include <string.h>
#include "apidef.h"

#ifndef CONFIG_PIN_SCL
#    define CONFIG_PIN_SCL 6
#endif
#ifndef CONFIG_PIN_SDA
#    define CONFIG_PIN_SDA 5
#endif

#ifndef CONFIG_I2C_FREQ
#    define CONFIG_I2C_FREQ I2C_FREQ_400K
#endif

#ifndef CONFIG_I2C_ADDR1
#    define CONFIG_I2C_ADDR1 0x21
#endif

#ifndef CONFIG_I2C_ADDR2
#    define CONFIG_I2C_ADDR2 0x10
#endif

#ifndef I2C_MAX_DATA_LEN
#    define I2C_MAX_DATA_LEN 2048
#endif

#ifndef I2C_7BIT_ADDR
#    define I2C_7BIT_ADDR(addr) (addr << 1)
#endif

#ifndef I2C_8BIT_ADDR
#    define I2C_8BIT_ADDR(addr) (addr)
#endif

typedef uint8_t i2c_status_t;

#define I2C_STATUS_SUCCESS (0)
#define I2C_STATUS_ERROR (1)
#define I2C_STATUS_TIMEOUT (1)

static inline int i2c_init(void) {
    const bmp_api_i2cm_config_t config = {
        .freq = CONFIG_I2C_FREQ,
        .scl  = CONFIG_PIN_SCL,
        .sda  = CONFIG_PIN_SDA,
    };
    return BMPAPI->i2cm.init(&config);
}

static inline void i2c_uninit(void) {
    BMPAPI->i2cm.uninit();
}

static inline uint8_t i2c_transmit(uint8_t address, const uint8_t* data, uint16_t length, uint16_t timeout) {
    uint8_t  i2c_temporary_buffer[length];
    uint8_t* p_send;

    if (data <= (uint8_t*)0xFFFFF) {
        // copy data on ROM to RAM
        if (sizeof(i2c_temporary_buffer) / sizeof(i2c_temporary_buffer[0]) < length) return 1;
        memcpy(i2c_temporary_buffer, data, length);
        p_send = i2c_temporary_buffer;
    } else {
        p_send = (uint8_t*)data;
    }

    return BMPAPI->i2cm.transmit(address >> 1, (uint8_t*)p_send, length);
}

static inline uint8_t i2c_receive(uint8_t address, uint8_t* data, uint16_t length, uint16_t timeout) {
    return BMPAPI->i2cm.receive(address >> 1, data, length);
}

static inline uint8_t i2c_readReg(uint8_t devaddr, uint8_t regaddr, uint8_t* data, uint16_t length, uint16_t timeout) {
    return BMPAPI->i2cm.read_reg(devaddr >> 1, regaddr, data, length, timeout);
}

static inline uint8_t i2c_write_register(uint8_t devaddr, uint8_t regaddr, const uint8_t* data, uint16_t length, uint16_t timeout) {
    uint8_t  i2c_temporary_buffer[length];
    uint8_t* p_send;

    if (data <= (uint8_t*)0xFFFFF) {
        // copy data on ROM to RAM
        if (sizeof(i2c_temporary_buffer) / sizeof(i2c_temporary_buffer[0]) < length) return 1;
        memcpy(i2c_temporary_buffer, data, length);
        p_send = i2c_temporary_buffer;
    } else {
        p_send = (uint8_t*)data;
    }

    return BMPAPI->i2cm.write_reg(devaddr >> 1, regaddr, p_send, length, timeout);
}

static inline uint8_t i2c_write_register16(uint8_t devaddr, uint16_t regaddr16, const uint8_t* data, uint16_t length, uint16_t timeout) {
    uint8_t i2c_temporary_buuffer[length + 2];
    i2c_temporary_buuffer[0] = regaddr16 >> 8;
    i2c_temporary_buuffer[1] = regaddr16 & 0xFF;
    for (uint16_t i = 0; i < length; i++) {
        i2c_temporary_buuffer[i + 2] = data[i];
    }

    return i2c_transmit(devaddr, i2c_temporary_buuffer, length + 2, timeout);
}

static inline uint8_t i2c_readReg16(uint8_t devaddr, uint16_t regaddr16, uint8_t* data, uint16_t length, uint16_t timeout) {
    uint8_t regaddr[2];
    regaddr[0] = regaddr16 >> 8;
    regaddr[1] = regaddr16 & 0xFF;
    i2c_transmit(devaddr, regaddr, 2, timeout);
    return i2c_receive(devaddr, data, length, timeout);
}