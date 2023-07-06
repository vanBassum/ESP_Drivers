#pragma once
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "device.h"
#include <vector>


class SPIDevice;

class SPIBus
{
	std::vector<SPIDevice> devices;
	spi_host_device_t host;
	spi_bus_config_t config;
	int dmaChannel;
	
public:
	SPIBus(spi_host_device_t _host, spi_bus_config_t _config, int _dmaChannel);
	~SPIBus();
	SPIDevice& addDevice(const spi_device_interface_config_t& devConfig);
};




