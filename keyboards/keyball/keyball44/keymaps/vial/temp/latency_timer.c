
// latency_timer.c
#include "latency_timer.h"
#include "hardware/timer.h"   // ここだけに閉じ込める

uint32_t latency_timer_now_us(void) {
    return time_us_32();
}