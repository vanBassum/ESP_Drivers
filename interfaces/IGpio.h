#pragma once
#include "IDevice.h"

class IGpio : public IDevice {
public:
    virtual ~IGpio() {}

    //https://docs.zephyrproject.org/latest/hardware/peripherals/gpio.html#c.GPIO_INPUT
    enum GpioFlags        
    {
        GPIO_FLAGS_INPUT  = 0x01,
        GPIO_FLAGS_OUTPUT = 0x02,
        //Opted not to include the rest yet, so we can divert if needed
    };

    
    // https://docs.zephyrproject.org/latest/hardware/peripherals/gpio.html#c.gpio_pin_configure
    // https://docs.zephyrproject.org/latest/hardware/peripherals/gpio.html#c.gpio_port_get_raw
    
    virtual DeviceResult portConfigure(uint32_t port, uint8_t mask, GpioFlags flags) = 0;
    virtual DeviceResult portRead(uint32_t port, uint8_t mask, uint8_t* value) = 0;
    virtual DeviceResult portWrite(uint32_t port, uint8_t mask, uint8_t value) = 0;


    // No reason to implement these
    // virtual ErrCode pinConfigure(uint32_t port, uint8_t pin, GpioFlags value) = 0;
    // virtual ErrCode pinRead(uint32_t port,      uint8_t pin, bool* value) = 0;
    // virtual ErrCode pinWrite(uint32_t port,     uint8_t pin, bool value) = 0;

    
    // TODO: https://docs.zephyrproject.org/latest/hardware/peripherals/gpio.html#c.gpio_pin_interrupt_configure
};

