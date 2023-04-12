#include "device.h"

esp_err_t SPI::Device::Init(Bus* bus, spi_device_interface_config_t* config)
{
	return spi_bus_add_device(bus->_host, config, &handle);
}



esp_err_t SPI::Device::PollingTransmit(spi_transaction_t* transaction)
{
	return spi_device_polling_transmit(handle, transaction);  	//Transmit!
}


esp_err_t SPI::Device::Transmit(spi_transaction_t* transaction)
{
	return spi_device_transmit(handle, transaction);
}




void SPI::Device::ReleaseBus()
{
	spi_device_release_bus(handle);
}


esp_err_t SPI::Device::AcquireBus()
{
	return spi_device_acquire_bus(handle, portMAX_DELAY);
}
