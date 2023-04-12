#include "device.h"

bool SPI::Device::Init(Bus* bus, spi_device_interface_config_t* config)
{
	return spi_bus_add_device(bus->_host, config, &handle) == ESP_OK;
}



bool SPI::Device::PollingTransmit(spi_transaction_t* transaction)
{
	return spi_device_polling_transmit(handle, transaction) == ESP_OK;  	//Transmit!
}


bool SPI::Device::Transmit(spi_transaction_t* transaction)
{
	return spi_device_transmit(handle, transaction) == ESP_OK;
}




void SPI::Device::ReleaseBus()
{
	spi_device_release_bus(handle);
}


bool SPI::Device::AcquireBus()
{
	return spi_device_acquire_bus(handle, portMAX_DELAY) == ESP_OK;
}
