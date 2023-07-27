#pragma once
#include <vector>
#include "driver/i2c.h"
#include "device.h"

class I2CBus
{
	const char* TAG = "I2CBus";
	std::vector<I2CDevice*> devices;
	i2c_port_t host;
	friend I2CDevice;
public:
	I2CBus(i2c_port_t host, const i2c_config_t& config);
	~I2CBus();
	
	I2CDevice& CreateDevice(const uint16_t device_address);
	
};

