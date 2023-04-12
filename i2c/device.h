#pragma once


#include "driver/i2c.h"


namespace I2C
{
    class Bus;
	class Device
	{
        Bus* bus;
        uint16_t address;
	public:
		bool Init(Bus* bus, uint16_t device_address);
        esp_err_t ReadWrite(const uint8_t *write_buffer, size_t write_size, uint8_t *read_buffer, size_t read_size);
        esp_err_t Write(const uint8_t *write_buffer, size_t write_size);
	};

}




