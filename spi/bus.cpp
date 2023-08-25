#include "bus.h"
#include "device.h"
#include "esp_log.h"

SPIBus::SPIBus(spi_host_device_t _host, spi_bus_config_t& _config, int _dmaChannel) 
	: host(_host)
	, config(_config)
	, dmaChannel(_dmaChannel)
{
	ESP_LOGI(TAG, "Initializing");
	ESP_ERROR_CHECK(spi_bus_initialize(host, &config, dmaChannel));
}
	
SPIBus::~SPIBus()
{
	devices.clear(); //By calling devices.clear() in the destructor, it will remove all the SPIDevice objects from the vector, triggering their destructors and ensuring proper cleanup.
	spi_bus_free(host);
}
	
SPIDevice& SPIBus::CreateDevice(const spi_device_interface_config_t& devConfig)
{
	devices.emplace_back(new SPIDevice(*this, devConfig));
	return *devices.back();
}