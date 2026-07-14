#pragma once

#include <stdint.h>

typedef union {
    uint32_t raw;
    struct {
        uint8_t  m_term;
        uint8_t  hue;
        uint8_t  Reserved1;
        uint8_t  Reserved2;
    };
} user_config_t;

void user_config_init(void);
void save_user_config(void);
