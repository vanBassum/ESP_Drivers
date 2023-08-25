#pragma once
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include <vector>
#include "device.h"

class SPIBus
{
	const char* TAG = "SPIBus";
	std::vector<SPIDevice*> devices;
	spi_host_device_t host;
	const spi_bus_config_t& config;
	int dmaChannel;
	
	friend SPIDevice;
public:
	SPIBus(spi_host_device_t _host, const spi_bus_config_t& _config, int _dmaChannel);
	~SPIBus();
	
	SPIDevice& CreateDevice(const spi_device_interface_config_t& devConfig);
	
};




