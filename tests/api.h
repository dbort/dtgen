#pragma once

#include <inttypes.h>
#include <assert.h>

//
// Global
//

//
// Portable API
//

// generic, uses uhal types

typedef struct {
  gpio_pin_t pin;
} board_signal_info_t;

typedef struct {
  i2c_t bus;
  uint8_t addr;
} board_i2c_peripheral_info_t;

// BSP interface only; internals not exposed to consumer
typedef struct {
  // Table that maps board_signal_t to info entries.
  board_signal_info_t* signal_map;
  size_t signal_map_size;
  // Table that maps board_i2c_peripheral_t to info entries.
  board_i2c_peripheral_info_t* i2c_peripheral_map;
  size_t i2c_peripheral_map_size;
} board_mapping_t;

//
// Fixed for chip
//

// uhal

typedef enum {
  GPIO_0,
  GPIO_1,
  GPIO_2,
} gpio_pin_t;

// The type used by the drivers, so that users don't need to convert
typedef enum {
  BUS_I2C0,
  BUS_I2C1,
  BUS_I2C2,
  BUS_MAX,
} i2c_t;

//
// Fixed for product across board IDs, though not all may be mapped
// for a given ID.
//

// Common namespace of all signals connected directly to chip pins.
//? What about GPIO extenders?
typedef enum {
    SIGNAL_AMP_L_RESET_N,
    SIGNAL_AMP_R_RESET_N,
    SIGNAL_DEBUG_UART_TX,
    SIGNAL_DEBUG_UART_RX,
    SIGNAL_BUTTON_N,
    SIGNAL_POWER,
    SIGNAL_MAX,
    SIGNAL_UNKNOWN,
} board_signal_t;

// Set of I2C peripherals.
typedef enum {
    PERIPHERAL_AUDIO_AMP_L,
    PERIPHERAL_AUDIO_AMP_R,
    PERIPHERAL_MAX,
} board_i2c_peripheral_t;

// Depends on board ID

// Users know the semantics of the pin, whether it's an input or output, pulled up or not
// We'd only need to know it here if we wanted to auto-configure all pins,
// but the current pattern is to let the driver config the pins
static board_signal_info_t dev0_signals[SIGNAL_MAX] = {
    [SIGNAL_AMP_L_RESET_N] = { .pin = GPIO_0 },
    [SIGNAL_AMP_R_RESET_N] = { .pin = GPIO_1 },
    [SIGNAL_DEBUG_UART_RX] = { .pin = GPIO_2 },
};

static board_i2c_peripheral_info_t dev0_i2c_peripherals[PERIPHERAL_MAX] = {
    [PERIPHERAL_AUDIO_AMP_L] = { .bus = BUS_I2C0, .addr = 0x45 },
    [PERIPHERAL_AUDIO_AMP_R] = { .bus = BUS_I2C2, .addr = 0x20 },
};

static board_mapping_t dev0_board_mapping = {
  .signal_map = dev0_signals,
  .signal_map_size = SIGNAL_MAX,
  .i2c_peripheral_map = dev0_i2c_peripherals,
  .i2c_peripheral_map_size = PERIPHERAL_MAX,
};

//
// API impl
//

// Internal BSP-defined function
const board_mapping_t* bsp_get_board_mapping() {
  return &dev0_board_mapping;
}

int board_get_signal(board_signal_t signal, board_signal_info_t* out_info) {
  if (out_info == NULL) {
    return -1;
  }
  // Another option would be for the caller to get and pass in the board mapping,
  // but that logic would always be the same. It would be more testable to inject
  // it, but we can easily override this function in testing since it's implemented
  // by the bsp.
  const board_mapping_t* map = bsp_get_board_mapping();
  if (map == NULL || signal < 0 || signal >= map->signal_map_size) {
    return -1;
  }

  // Having the user provide the struct lets the internal mapping use a
  // different layout if desired.
  *out_info = map->signal_map[signal];
  return 0;
}

int board_get_i2c_peripheral(board_i2c_peripheral_t peripheral, board_i2c_peripheral_info_t* out_info) {
  if (out_info == NULL) {
    return -1;
  }
  const board_mapping_t* map = bsp_get_board_mapping();
  if (map == NULL || peripheral < 0 || peripheral >= map->i2c_peripheral_map_size) {
    return -1;
  }

  // Having the user provide the struct lets the internal mapping use a
  // different layout if desired, at the expense of a copy.
  *out_info = map->i2c_peripheral_map[peripheral];
  return 0;
}

//
// User code
//

int audio_amp_init() {
  int status;
  board_i2c_peripheral_info_t amp_l;
  status = board_get_i2c_peripheral(PERIPHERAL_AUDIO_AMP_L, &amp_l);
  if (status != 0) {
    return status;
  }
  // Can use amp_l.bus and amp_l.addr to talk to the peripheral.

  board_i2c_peripheral_info_t amp_r;
  status = board_get_i2c_peripheral(PERIPHERAL_AUDIO_AMP_R, &amp_r);
  if (status != 0) {
    return status;
  }

  board_signal_info_t amp_l_reset_n;
  status = board_get_signal(SIGNAL_AMP_L_RESET_N, &amp_l_reset_n);
  if (status != 0) {
    return status;
  }
  // Can use amp_l_reset_n.pin to configure and use the signal.

  board_signal_info_t amp_r_reset_n;
  status = board_get_signal(SIGNAL_AMP_L_RESET_N, &amp_l_reset_n);
  if (status != 0) {
    return status;
  }
}

//?? Multiple signals may map to a single pin, especially if there are jumpers
//   Or if the signal changes at runtime depending on the state (camera vs mic)
// Can a single signal map to different pins at runtime? For now assume that it's
//   fixed once we know the board ID

/*
- Get the pin that the signal maps to -- more for debugging?
  - uhal currently requires putting the pins in the init config, so we'd need to
    get the pin for a given signal

ENABLING: that's a separate layer. Here we just want to represent the hardware, providing
enough read-only information to reflect the schematic that the software is running on.
A higher layer could manage enabling, low-power modes etc. This lower layer could include
metadata to help with that ("boot state", "low power state") but doesn't change anything
on its own.

If we generate these mappings from a spreadsheet, we could generate a different table of
states to set pins to for major system transitions: boot, sleep. That data doesn't need
to be visible to users of the board API.

But it could be stored in the same internal tables, we just wouldn't expose it to users.
*/