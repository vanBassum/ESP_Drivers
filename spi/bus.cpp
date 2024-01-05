#include "bus.h"
#include "esp_log.h"
#include "driver/spi_master.h"

void SPIBus::init()
{
	assert(initialized == 0 && "SPIBus already initialized");
	ESP_LOGI(TAG, "Initializing host %d", config.host);
	ESP_ERROR_CHECK(spi_bus_initialize(config.host, &config.config, config.dmaChannel));
	initialized = true;
}

bool SPIBus::isInitialized() const
{
    return initialized;
}

void SPIBus::setConfig(const Config &newConfig)
{
	assert(initialized == 0 && "Can't change configuration after initialisation");
	config = newConfig;
}
