#pragma once

#include <inttypes.h>
#include <assert.h>

// Fixed for chip

// 3 bits?
typedef enum {
    BUS_TYPE_I2C,
    BUS_TYPE_I3C,
    BUS_TYPE_SPI,
    BUS_TYPE_1W,
    BUS_TYPE_USB,
    BUS_TYPE_MAX,
} bus_type_t;

#define DECLARE_BUS(type, index) ((type)<<4 | (index))
#define GET_BUS_TYPE(bus) ((bus)>>4)
#define GET_BUS_INDEX(bus) ((bus)&0xf)

typedef enum {
    BUS_I2C0 = DECLARE_BUS(BUS_TYPE_I2C, 0),
    BUS_I2C1 = DECLARE_BUS(BUS_TYPE_I2C, 1),
    BUS_I2C2 = DECLARE_BUS(BUS_TYPE_I2C, 2),
    BUS_SPI0 = DECLARE_BUS(BUS_TYPE_SPI, 0),
    BUS_MAX,
} bsp_bus_t;

// Fixed for product across board IDs, though not all may be mapped
// for a given ID.

// Common namespace of all signals, GPIOs or otherwise.
//? What about GPIO extenders?
typedef enum {
    SIGNAL_AMP_L_RESET, // Skip the _N since ACTIVE_LOW takes care of it
    SIGNAL_AMP_R_RESET, // Skip the _N since ACTIVE_LOW takes care of it
    SIGNAL_DEBUG_UART_TX,
    SIGNAL_DEBUG_UART_RX,
    SIGNAL_BUTTON_N,
    SIGNAL_POWER,
    SIGNAL_MOTOR_SS, // Skip the _N since ACTIVE_LOW takes care of it
    SIGNAL_MAX,
    SIGNAL_UNKNOWN,
} bsp_signal_t;

// Set of off-chip peripherals. Could specialize to a given bus
// type, since we're not going to transparently move a peripheral
// from i2c to SPI, but it's also ok to mix all names here and
// leave that to the API level.
typedef enum {
    PERIPHERAL_AUDIO_AMP_L,
    PERIPHERAL_AUDIO_AMP_R,
    PERIPHERAL_MOTOR,
    PERIPHERAL_MAX,
} bsp_peripheral_t;

// Depends on board ID

typedef enum {
  GPIO_0,
  GPIO_1,
  GPIO_2,
} gpio_pin_t;

typedef struct {
    gpio_pin_t pin;
    uint32_t flags; // in vs out vs special; pull up/down/none; 
} bsp_signal_desc_t;

// include/dt-bindings/gpio/gpio.h
// https://elinux.org/images/a/a7/ELC-2021_Introduction_to_pin_muxing_and_GPIO_control_under_Linux.pdf

/* Bit 0 express polarity */
#define GPIO_ACTIVE_HIGH (0<<0)
#define GPIO_ACTIVE_LOW (1<<0)
/* Bit 4 express pull up */
#define GPIO_PULL_UP (1<<4) 
/* Bit 5 express pull down */
#define GPIO_PULL_DOWN (1<<5) 

// Users know the semantics of the pin, whether it's an input or output
// We'd only need to know it here if we wanted to auto-configure all pins,
// but the current pattern is to let the driver config the pins
static bsp_signal_desc_t dev0_signals[SIGNAL_MAX] = {
    [SIGNAL_AMP_L_RESET] = { .pin = GPIO_0, .flags = GPIO_ACTIVE_LOW | GPIO_PULL_UP },
    [SIGNAL_AMP_R_RESET] = { .pin = GPIO_1, .flags = GPIO_ACTIVE_LOW | GPIO_PULL_UP },
    [SIGNAL_POWER] = { .pin = GPIO_2, .flags = GPIO_ACTIVE_HIGH | GPIO_PULL_UP },
};

typedef struct {
    bsp_bus_t bus;
    union {
      uint8_t i2c_addr;
      bsp_signal_t spi_ss_signal;
    };
} bsp_peripheral_desc_t;

// why mix buses? callers will know which type of bus a thing is on
static bsp_peripheral_desc_t dev0_peripherals[PERIPHERAL_MAX] = {
    [PERIPHERAL_AUDIO_AMP_L] = { .bus = BUS_I2C0, .i2c_addr = 0x45 },
    [PERIPHERAL_AUDIO_AMP_R] = { .bus = BUS_I2C2, .i2c_addr = 0x20 },
    [PERIPHERAL_MOTOR] = { .bus = BUS_SPI0, .spi_ss_signal = SIGNAL_MOTOR_SS },
};

typedef struct {
    bsp_signal_t signal;
    gpio_pin_t pin;
    // Three lists? inputs, outputs, special?
} bsp_signal_mapping_t;

// Usage

typedef void bsp_signal_map_t;

bsp_signal_map_t* bsp_get_signal_map() {
  // Check boardID
  return dev0_signals;
}

bsp_signal_desc_t* bsp_get_signal(bsp_signal_map_t* pmap, bsp_signal_t signal) {
  if (pmap == NULL || signal < 0 || signal >= SIGNAL_MAX) {
    return NULL;
  }
  return ((bsp_signal_desc_t*)pmap) + signal;
}

typedef void bsp_peripheral_map_t;

bsp_peripheral_map_t* bsp_get_peripheral_map() {
  // Check boardID
  return dev0_peripherals;
}

bsp_peripheral_desc_t* bsp_get_peripheral(bsp_peripheral_map_t* pmap, bsp_peripheral_t peripheral) {
  if (pmap == NULL || peripheral < 0 || peripheral >= PERIPHERAL_MAX) {
    return NULL;
  }
  return ((bsp_peripheral_desc_t*)pmap) + peripheral;
}

int audio_amp_init() {
  bsp_peripheral_map_t* pmap = bsp_get_peripheral_map();
  bsp_peripheral_desc_t* amp_l = bsp_get_peripheral(pmap, PERIPHERAL_AUDIO_AMP_L);
  bsp_peripheral_desc_t* amp_r = bsp_get_peripheral(pmap, PERIPHERAL_AUDIO_AMP_R);
  if (amp_l == NULL || amp_r == NULL) {
    return -1;
  }
  // awkward if amp_l->bus isn't compatible with uhal i2c enum
  // could have a macro to convert, and sneak the uhal enum into the bus enum
  // or have different types/functions per bus

  bsp_signal_map_t* smap = bsp_get_signal_map();
  bsp_signal_desc_t* amp_l_reset = bsp_get_signal(smap, SIGNAL_AMP_L_RESET);
  bsp_signal_desc_t* amp_r_reset = bsp_get_signal(smap, SIGNAL_AMP_R_RESET);

#if 0
  // if we're using ACTIVE_LOW, ideally some underlying API would translate that for us
  // Maybe there's a signal config/read/write layer above the gpio layer
  // or we leave it to the user
  gpio_out_cfg(amp_l_reset->pin);
  if (amp_l_reset->flags & GPIO_ACTIVE_LOW) {
    gpio_write(amp_l_reset->pin, 1);
  } else {
    gpio_write(amp_l_reset->pin, 0);
  }
#endif
}

// Bus addresses:
// - Also need a peripheral map with i2c bus and address, since that can
//   change between board revisions. Same for i3c, SPI, etc. Or even 1wire pins
// - i2c bus metadata? app or sensor controls a bus? That could be in code to start with

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

- Enable a signal:
  - Set up its pin. It might override another signal that's currently using the same pin,
    but that might be up to the higher levels to worry about. Or we could keep a map
    of whether a given pin is enabled, and fail if we try to re-enable it
    - "set up its pin": configure it to its initial state

- Put system in a different state: low-power, boot, default, custom
  - Moving in and out of low-power: do we record the previous state so we can restore it?
    Or require that all drivers have disabled their signals at that point?

- I'm the i2c driver and someone just told me to enable bus 0
  - uhal currently requires putting the pins in the init config, so we'd need to
    get the pins for that bus
  - could have a custom "i2c pin group" struct, but then would also need custom pin
    groups for every type of peripheral
  - start out by having signals that map all pins: I2C0_SDA, I2C0_SCL
  - i2c driver knows that it should configure those pins as a specific function with
    appropriate pullup/down, so we don't need that in the pin map
*/