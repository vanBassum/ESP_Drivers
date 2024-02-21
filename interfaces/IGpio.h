#pragma once
#include "IDevice.h"
#include <functional>


#define GPIO_CREATE_CONFIG(mode, intr, pull) \
    {                                         \
        mode,                         \
        intr,                         \
        pull                          \
    }

// Here I opted to mimic the ESP way.
enum GpioConfigMode : uint8_t
{
    GPIO_CFG_MODE_DISABLE = 0x00,         /*!< GPIO mode : disable input and output               */
    GPIO_CFG_MODE_INPUT = 0x01,           /*!< GPIO mode : input only                              */
    GPIO_CFG_MODE_OUTPUT = 0x02,          /*!< GPIO mode : output only mode                        */
    GPIO_CFG_MODE_OUTPUT_OD = 0x03,       /*!< GPIO mode : output only with open-drain mode        */
    GPIO_CFG_MODE_INPUT_OUTPUT_OD = 0x04, /*!< GPIO mode : output and input with open-drain mode  */
    // GPIO_CFG_MODE_INPUT_OUTPUT      = 0x05,   /*!< GPIO mode : output and input mode                   */
};

enum GpioConfigIntr : uint8_t
{
    GPIO_CFG_INTR_DISABLE = 0x00,    /*!< Disable GPIO interrupt                             */
    GPIO_CFG_INTR_POSEDGE = 0x01,    /*!< GPIO interrupt type : rising edge                  */
    GPIO_CFG_INTR_NEGEDGE = 0x02,    /*!< GPIO interrupt type : falling edge                 */
    GPIO_CFG_INTR_ANYEDGE = 0x03,    /*!< GPIO interrupt type : both rising and falling edge */
    GPIO_CFG_INTR_LOW_LEVEL = 0x04,  /*!< GPIO interrupt type : input low level trigger      */
    GPIO_CFG_INTR_HIGH_LEVEL = 0x05, /*!< GPIO interrupt type : input high level trigger     */
};

enum GpioConfigPull : uint8_t
{
    GPIO_CFG_PULL_DISABLE = 0x00,    /*!< Enable GPIO pull-up resistor                        */
    GPIO_CFG_PULLUP_ENABLE = 0x01,   /*!< Enable GPIO pull-up resistor                        */
    GPIO_CFG_PULLDOWN_ENABLE = 0x02, /*!< Enable GPIO pull-down resistor                      */
};

struct GpioConfig
{
    GpioConfigMode mode;
    GpioConfigIntr intr;
    GpioConfigPull pull;
};

class IGpio : public IDevice
{
public:
    virtual ~IGpio() {}

    

    // https://docs.zephyrproject.org/latest/hardware/peripherals/gpio.html#c.GPIO_INPUT
    // https://docs.zephyrproject.org/latest/hardware/peripherals/gpio.html#c.gpio_pin_configure
    // https://docs.zephyrproject.org/latest/hardware/peripherals/gpio.html#c.gpio_port_get_raw
    // https://docs.zephyrproject.org/latest/hardware/peripherals/gpio.html#c.gpio_pin_interrupt_configure

    virtual Result GpioConfigure(uint32_t port, uint8_t mask, const GpioConfig* config) = 0;
    virtual Result GpioRead(uint32_t port, uint8_t mask, uint8_t *value) = 0;
    virtual Result GpioWrite(uint32_t port, uint8_t mask, uint8_t value) = 0;
    virtual Result GpioIsrAddCallback(uint32_t port, uint8_t pin, std::function<void()> callback) { return Result::NotSupported; }
    virtual Result GpioIsrRemoveCallback(uint32_t port, uint8_t pin) { return Result::NotSupported; }
};
