// Copyright 2023 sekigon-gonnoc
/* SPDX-License-Identifier: GPL-2.0-or-later */

#include <string.h>

#include "timer.h"
#include "wait.h"

#include "xmodem.h"
#include "apidef.h"

enum {
    SOH = 0x01,
    STX = 0x02,
    ETX = 0x03,
    EOT = 0x04,
    ACK = 0x06,
    NAK = 0x15,
    CAN = 0x18,
};

typedef enum {
    XMODEM_IDLE,
    XMODEM_RECEIVING_PACKET,
    XMODEM_WAIT_NEXT_PACKET,
    XMODEM_COMPLETE,
} xmodem_state_id_t;

typedef struct {
    volatile xmodem_state_id_t id;
    volatile uint16_t          packet_write_idx;
    void              (*complete_callback)(void);
    void              (*packet_receive_callback)(xmodem_packet_t *packet);
} xmodem_state_t;

static xmodem_state_t xmodem_state;
static xmodem_packet_t xmodem_packet;

static inline void send_data(uint8_t d) {
    BMPAPI->usb.serial_putc(d);
}

static inline char get_data(void) {
    return BMPAPI->usb.serial_getc();
}

static inline int32_t get_byte_to_read(void) {
    return BMPAPI->usb.serial_byte_to_read();
}

static void request_data(void) {
    xmodem_state.packet_write_idx = 0;
    send_data('C');
}

static void send_ack(void) {
    xmodem_state.packet_write_idx = 0;
    send_data(ACK);
}

static void receive_header(void) {
    int32_t btr = get_byte_to_read();

    while (btr--) {
        char c = get_data();
        if (c == SOH) {
            xmodem_state.id               = XMODEM_RECEIVING_PACKET;
            xmodem_packet.bytes[0]        = c;
            xmodem_state.packet_write_idx = 1;

            return;
        } else if (c == EOT) {
            xmodem_state.id = XMODEM_COMPLETE;

            if (xmodem_state.complete_callback != NULL) {
                xmodem_state.complete_callback();
            }

            send_ack();

            return;
        } else if (c == ETX) {
            // terminate
            xmodem_state.id = XMODEM_COMPLETE;

            if (xmodem_state.complete_callback != NULL) {
                xmodem_state.complete_callback();
            }

            return;
        }
    }
}

static void xmodem_task_idle(void) {
    receive_header();

    if (xmodem_state.id == XMODEM_IDLE) {
        static uint32_t prev_request_time;
        if (timer_elapsed32(prev_request_time) > 1000) {
            request_data();
            prev_request_time = timer_read32();
        }
    }
}

static void xmodem_task_receive(void) {
    int32_t btr = get_byte_to_read();

    while (btr--) {
        char c = get_data();

        xmodem_packet.bytes[xmodem_state.packet_write_idx++] = c;

        if (xmodem_state.packet_write_idx >= sizeof(xmodem_packet)) {
            if (xmodem_state.packet_receive_callback != NULL) {
                xmodem_state.packet_receive_callback(&xmodem_packet);
            }

            send_data(ACK);
            
            xmodem_state.id = XMODEM_WAIT_NEXT_PACKET;

            return;
        }
    }
}

static void xmodem_task_wait_next_packet(void) {
    receive_header();

    // if (xmodem_state.id == XMODEM_WAIT_NEXT_PACKET) {
    //     static uint32_t prev_request_time;
    //     if (timer_elapsed32(prev_request_time) > 5000) {
    //         send_ack();
    //         prev_request_time = timer_read32();
    //     }
    // }
}

void xmodem_init(void (*complete_callback)(void), void (*packet_receive_callback)(xmodem_packet_t *)) {
    memset(&xmodem_state, 0, sizeof(xmodem_state));
    xmodem_state.complete_callback       = complete_callback;
    xmodem_state.packet_receive_callback = packet_receive_callback;
}

void xmodem_task(void *_) {
    if (xmodem_state.id == XMODEM_IDLE) {
        xmodem_task_idle();
    }
    else if (xmodem_state.id == XMODEM_RECEIVING_PACKET) {
        xmodem_task_receive();
    }
    else if (xmodem_state.id == XMODEM_WAIT_NEXT_PACKET) {
        xmodem_task_wait_next_packet();
    }
}
