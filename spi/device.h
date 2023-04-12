#pragma once
#include "driver/spi_master.h"
#include "bus.h"

namespace ESP_Com
{
	class SPIBus;
	class SPIDevice
	{
		spi_device_handle_t handle = NULL;
	public:
	
		esp_err_t Init(SPIBus* bus, spi_device_interface_config_t* config);
		esp_err_t PollingTransmit(spi_transaction_t* transaction);
		esp_err_t Transmit(spi_transaction_t* transaction);
		esp_err_t AcquireBus();
		void ReleaseBus();
	};
}

