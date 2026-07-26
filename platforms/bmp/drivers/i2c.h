
#pragma once
#include <stdint.h>
#include "i2c_master.h"


static inline int i2cs_init(void)
{
  bmp_api_i2cs_config_t config = {
    .sda = CONFIG_PIN_SDA,
    .scl = CONFIG_PIN_SCL,
    .address = {CONFIG_I2C_ADDR1, CONFIG_I2C_ADDR2},
  };
  return BMPAPI->i2cs.init(&config);
}

static inline void i2cs_uninit(void)
{
  BMPAPI->i2cs.uninit();
}

static inline void i2cs_prepare(uint8_t* const dat, uint8_t len)
{
  BMPAPI->i2cs.prepare(dat, len);
}
