#pragma once
#include "driver/i2c.h"
#include "device.h"

namespace I2C
{
    class Device;
    class Bus
    {
    protected:
      i2c_port_t host;
      friend Device;
    public:
      esp_err_t Init(i2c_port_t host, gpio_num_t scl, gpio_num_t sda);
    };

}


