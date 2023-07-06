#include "bus.h"
#include "device.h"

SPIBus::SPIBus(spi_host_device_t _host, spi_bus_config_t _config, int _dmaChannel) 
	: host(_host)
	, config(_config)
	, dmaChannel(_dmaChannel)
{
	config.flags = 0;
	config.intr_flags = 0;
	ESP_ERROR_CHECK(spi_bus_initialize(host, &config, dmaChannel));
}
	
SPIBus::~SPIBus()
{
	devices.clear(); //By calling devices.clear() in the destructor, it will remove all the SPIDevice objects from the vector, triggering their destructors and ensuring proper cleanup.
	spi_bus_free(host);
}
	
SPIDevice& SPIBus::addDevice(const spi_device_interface_config_t& devConfig)
{
	devices.emplace_back(host, devConfig);
	return devices.back();
}
