#pragma once

#include <inttypes.h>

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
    SIG_UNKNOWN,
    SIG_USB_RESET_N,
    SIG_DEBUG_UART_TX,
    SIG_DEBUG_UART_RX,
    SIG_BUTTON_N,
    SIG_MAX,
} bsp_signal_t;

// Set of off-chip peripherals. Could specialize to a given bus
// type, since we're not going to transparently move a peripheral
// from i2c to SPI, but it's also ok to mix all names here and
// leave that to the API level.
typedef enum {
    PERIPHERAL_AUDIO_AMP_L,
    PERIPHERAL_AUDIO_AMP_R,
} bsp_peripheral_t;

// Depends on board ID

typedef struct {
    bsp_peripheral_t peripheral;
    bsp_bus_t bus;
    uint32_t address;
} bsp_peripheral_mapping_t;

static bsp_peripheral_mapping_t dev0_peripherals[] = {
    {.peripheral = PERIPHERAL_AUDIO_AMP_L, .bus = BUS_I2C0, .address = 0x45},
    {.peripheral = PERIPHERAL_AUDIO_AMP_R, .bus = BUS_I2C2, .address = 0x20},
};

typedef uint32_t gpio_pin_t;

typedef struct {
    bsp_signal_t signal;
    gpio_pin_t pin;
    // Three lists? inputs, outputs, special?
} bsp_signal_mapping_t;

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