#include "device.h"
#include "bus.h"

SPIDevice::SPIDevice(SPIBus& bus, const spi_device_interface_config_t& devConfig)
{
	ESP_ERROR_CHECK(spi_bus_add_device(bus.host, &devConfig, &spi));
}
	
void SPIDevice::transfer(uint8_t* txData, uint8_t* rxData, size_t length)
{
	spi_transaction_t transaction {}
	;
	transaction.length = length * 8; // In bits
	transaction.tx_buffer = txData;
	transaction.rx_buffer = rxData;
	ESP_ERROR_CHECK(spi_device_transmit(spi, &transaction));
}

void SPIDevice::PollingTransmit(spi_transaction_t* transaction)
{
	esp_err_t res = spi_device_polling_transmit(spi, transaction);
	ESP_ERROR_CHECK(res);  	//Transmit!
}

void SPIDevice::Transmit(spi_transaction_t* transaction)
{
	ESP_ERROR_CHECK(spi_device_transmit(spi, transaction) ) ;
}

void SPIDevice::ReleaseBus()
{
	spi_device_release_bus(spi);
}

void SPIDevice::AcquireBus()
{
	ESP_ERROR_CHECK(spi_device_acquire_bus(spi, portMAX_DELAY));
}
