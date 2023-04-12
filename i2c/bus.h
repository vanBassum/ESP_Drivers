#pragma once
#include "driver/i2c.h"
#include "device.h"

namespace ESP_Drivers
{
    class I2CDevice;
    class I2CBus
    {
    protected:
      i2c_port_t host;
      friend I2CDevice;
    public:
      esp_err_t Init(i2c_port_t host, gpio_num_t scl, gpio_num_t sda);
    };

}


