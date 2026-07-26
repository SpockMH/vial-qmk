
#pragma once

#include <stdint.h>
#include "matrix.h"
#include "apidef.h"

typedef struct {
    void (*init)(void); // Initialize matrix
    uint32_t (*get_device_row)(void); // return matrix row count scanned in scan()
    uint32_t (*get_device_col)(void); // return matrix col count scanned in scan()
    uint32_t (*scan)(matrix_row_t *matrix_raw); // scan matrix and store to matrix_raw[]
} bmp_matrix_func_t;

typedef uint32_t matrix_col_t;

uint32_t bmp_matrix_get_device_row(void);