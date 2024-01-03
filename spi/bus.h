#pragma once
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include <vector>
#include <functional>

class SPIBus
{
	const char* TAG = "SPIBus";

public:
	spi_host_device_t host;
	spi_bus_config_t config;
	int dmaChannel;

	SPIBus(std::function<void(SPIBus& bus)> config);
	~SPIBus();



	esp_err_t add_device(const spi_device_interface_config_t* config, spi_device_handle_t* handle)
	{
		return spi_bus_add_device(host, config, handle);
	}

	esp_err_t device_transmit(spi_device_handle_t handle, spi_transaction_t* tx)
	{
		return spi_device_transmit(handle, tx);
	}

	esp_err_t device_polling_transmit(spi_device_handle_t handle, spi_transaction_t* tx)
	{
		return spi_device_polling_transmit(handle, tx);
	}

	void device_release_bus(spi_device_handle_t handle)
	{
		spi_device_release_bus(handle);
	}

	esp_err_t device_acquire_bus(spi_device_handle_t handle, TickType_t timeout)
	{
		return spi_device_acquire_bus(handle, timeout);
	}




};




