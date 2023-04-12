#pragma once


#include "driver/i2c.h"


namespace ESP_Drivers
{
    class I2CBus;
	class I2CDevice
	{
        I2CBus* bus;
        uint16_t address;
	public:
		esp_err_t Init(I2CBus* bus, uint16_t device_address);
        esp_err_t ReadWrite(const uint8_t *write_buffer, size_t write_size, uint8_t *read_buffer, size_t read_size);
        esp_err_t Write(const uint8_t *write_buffer, size_t write_size);
	};

}



