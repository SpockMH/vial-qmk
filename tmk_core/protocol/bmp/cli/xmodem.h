// Copyright 2023 sekigon-gonnoc
/* SPDX-License-Identifier: GPL-2.0-or-later */

#pragma once

#include <stdint.h>

typedef union {
    uint8_t bytes[133];
    struct {
        uint8_t header;
        uint8_t block_no;
        uint8_t block_no_comp;
        uint8_t data[128];
        uint8_t checksum;
    };
} xmodem_packet_t;

void xmodem_init(void (*complete_callback)(void), void (*packet_receive_callback)(xmodem_packet_t *packet));
void xmodem_task(void *_);