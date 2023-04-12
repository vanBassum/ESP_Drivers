#include "bus.h"


bool I2C::Bus::Init(i2c_port_t host, gpio_num_t scl, gpio_num_t sda)
{
    this->host = host;
    i2c_config_t conf;
    conf.mode = I2C_MODE_MASTER;
    conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
    conf.master.clk_speed = 400000;
    conf.sda_io_num = sda;
    conf.scl_io_num = scl;
    conf.clk_flags = 0;
    i2c_param_config(host, &conf);
    return i2c_driver_install(host, conf.mode, 0, 0, 0) == ESP_OK;
}


