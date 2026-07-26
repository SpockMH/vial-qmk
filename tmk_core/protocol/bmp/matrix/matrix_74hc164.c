
#include "quantum.h"

#include "bmp_matrix.h"
#include "apidef.h"
#include "bmp.h"

static void                    init_row2col(void);
static void                    init_col2row(void);
static uint32_t                get_device_row(void);
static uint32_t                get_device_col(void);
static uint32_t                scan_row2col(matrix_row_t *);
static uint32_t                scan_col2row(matrix_row_t *);
extern const bmp_matrix_func_t matrix_func_row2col;
extern const bmp_matrix_func_t matrix_func_col2row;

const bmp_matrix_func_t matrix_func_74hc164row2col = {init_row2col, get_device_row, get_device_col, scan_row2col};
const bmp_matrix_func_t matrix_func_74hc164col2row = {init_col2row, get_device_row, get_device_col, scan_col2row};

static uint32_t get_device_row(void) {
    return bmp_config->matrix.device_rows;
}

static uint32_t get_device_col(void) {
    return bmp_config->matrix.device_cols;
}

//
//// col2row matrix
//
static void init_col2row(void) {
    const uint32_t SR_RST = bmp_config->matrix.row_pins[2];
    for (int col = 0; col < bmp_config->matrix.device_cols; col++) {
        gpio_set_pin_input_high(bmp_config->matrix.col_pins[col]);
    }
    gpio_write_pin_high(SR_RST);
    gpio_set_pin_output_push_pull(SR_RST);
}

static uint32_t scan_col2row(matrix_row_t *matrix_raw) {
    const uint32_t SR_DATA = bmp_config->matrix.row_pins[0];
    const uint32_t SR_CLK  = bmp_config->matrix.row_pins[1];
    const uint8_t  matrix_offset = bmp_config->matrix.is_left_hand ? 0 : bmp_config->matrix.rows - get_device_row();

    // set initial data
    gpio_write_pin_high(SR_DATA);
    wait_us(1);
    for (int row = 0; row < bmp_config->matrix.device_rows; row++) {
        gpio_write_pin_high(SR_CLK);
        gpio_write_pin_low(SR_CLK);
    }

    gpio_write_pin_low(SR_DATA);
    wait_us(1);
    gpio_write_pin_high(SR_CLK);
    gpio_write_pin_low(SR_CLK);
    gpio_write_pin_high(SR_DATA);

    matrix_row_t current_matrix[MATRIX_ROWS] = {0};

    for (int row = 0; row < bmp_config->matrix.device_rows; row++) {
        // read row
        for (int col = 0; col < bmp_config->matrix.device_cols; col++) {
            current_matrix[col + matrix_offset] |= (readPin(bmp_config->matrix.col_pins[col]) ? 0 : 1) << row;
        }

        // shift data
        gpio_write_pin_high(SR_CLK);
        gpio_write_pin_low(SR_CLK);
        wait_us(1);
    }

    uint32_t matrix_change = 0;
    for (int row = 0; row < bmp_config->matrix.device_rows; row++) {
        if (matrix_raw[row + matrix_offset] != current_matrix[row + matrix_offset]) {
            dprintf("row:%d %04lx\n", row, current_matrix[row + matrix_offset]);
            matrix_raw[row + matrix_offset] = current_matrix[row + matrix_offset];
            matrix_change   = 1;
        }
    }

    return matrix_change;
}

//
//// row2col matrix
//
static void init_row2col(void) {
    const uint32_t SR_RST = bmp_config->matrix.col_pins[2];
    for (int row = 0; row < bmp_config->matrix.device_rows; row++) {
        gpio_set_pin_input_high(bmp_config->matrix.row_pins[row]);
    }
    gpio_write_pin_high(SR_RST);
    gpio_set_pin_output_push_pull(SR_RST);
}

static uint32_t scan_row2col(matrix_row_t *matrix_raw) {
    const uint32_t SR_DATA = bmp_config->matrix.col_pins[0];
    const uint32_t SR_CLK  = bmp_config->matrix.col_pins[1];
    const uint8_t  matrix_offset = bmp_config->matrix.is_left_hand ? 0 : bmp_config->matrix.rows - get_device_row();

    // set initial data
    gpio_write_pin_high(SR_DATA);
    wait_us(1);
    for (int col = 0; col < bmp_config->matrix.device_cols; col++) {
        gpio_write_pin_high(SR_CLK);
        gpio_write_pin_low(SR_CLK);
    }

    gpio_write_pin_low(SR_DATA);
    wait_us(1);
    gpio_write_pin_high(SR_CLK);
    gpio_write_pin_low(SR_CLK);
    gpio_write_pin_high(SR_DATA);

    matrix_col_t current_matrix[MATRIX_ROWS] = {0};

    for (int col = 0; col < bmp_config->matrix.device_cols; col++) {
        // read col
        for (int row = 0; row < bmp_config->matrix.device_rows; row++) {
            current_matrix[row + matrix_offset] |= (readPin(bmp_config->matrix.row_pins[row]) ? 0 : 1) << col;
        }

        // shift data
        gpio_write_pin_high(SR_CLK);
        gpio_write_pin_low(SR_CLK);
        wait_us(1);
    }

    uint32_t matrix_change = 0;
    for (int col = 0; col < bmp_config->matrix.device_cols; col++) {
        if (matrix_raw[col + matrix_offset] != current_matrix[col + matrix_offset]) {
            dprintf("col:%d %04lx\n", col, current_matrix[col + matrix_offset]);
            matrix_raw[col + matrix_offset] = current_matrix[col + matrix_offset];
            matrix_change   = 1;
        }
    }

    return matrix_change;
}
