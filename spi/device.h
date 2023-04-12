#pragma once
#include "driver/spi_master.h"
#include "bus.h"

namespace SPI
{
	class Bus;
	class Device
	{
		spi_device_handle_t handle = NULL;
	public:
	
		bool Init(Bus* bus, spi_device_interface_config_t* config);
		bool PollingTransmit(spi_transaction_t* transaction);
		bool Transmit(spi_transaction_t* transaction);
		bool AcquireBus();
		void ReleaseBus();
	};
}

