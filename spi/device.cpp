#include "device.h"
#include "esp_log.h"


SPIDevice::SPIDevice(std::shared_ptr<SPIBus> spiBus, std::function<void(SPIDevice& dev)> configurator)
{
	configurator((*this));
	ESP_ERROR_CHECK(spiBus->add_device(&devConfig, &handle));
}
	

void SPIDevice::transfer(uint8_t* txData, uint8_t* rxData, size_t length)
{
	ContextLock lock(mutex);
	spi_transaction_t transaction {};
	transaction.length = length * 8; // In bits
	transaction.tx_buffer = txData;
	transaction.rx_buffer = rxData;
	ESP_ERROR_CHECK(spiBus->device_transmit(handle, &transaction));
}

void SPIDevice::PollingTransmit(spi_transaction_t* transaction)
{
	ContextLock lock(mutex);
	esp_err_t res = spiBus->device_polling_transmit(handle, transaction);
	ESP_ERROR_CHECK(res);  	//Transmit!
}

void SPIDevice::Transmit(spi_transaction_t* transaction)
{
	ContextLock lock(mutex);
	ESP_ERROR_CHECK(spiBus->device_transmit(handle, transaction) ) ;
}

void SPIDevice::ReleaseBus()
{
	ContextLock lock(mutex);
	spiBus->device_release_bus(handle);
}

void SPIDevice::AcquireBus()
{
	ContextLock lock(mutex);
	ESP_ERROR_CHECK(spiBus->device_acquire_bus(handle, portMAX_DELAY));
}
