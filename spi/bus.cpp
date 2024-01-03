#include "bus.h"
#include "esp_log.h"
#include "driver/spi_master.h"

SPIBus::SPIBus(std::function<void(SPIBus& bus)> configurator)
{
	configurator((*this));
	ESP_LOGI(TAG, "Initializing");
	ESP_ERROR_CHECK(spi_bus_initialize(host, &config, dmaChannel));
}
	
SPIBus::~SPIBus()
{
	spi_bus_free(host);
}
	
