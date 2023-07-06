#pragma once
#include "driver/spi_master.h"
#include "bus.h"

class SPIDevice
{
	spi_device_handle_t spi;
	SPIDevice(spi_host_device_t host, const spi_device_interface_config_t& devConfig);
   	
	friend class SPIBus;
public:
	void transfer(uint8_t* txData, uint8_t* rxData, size_t length);
	void PollingTransmit(spi_transaction_t* transaction);
	void Transmit(spi_transaction_t* transaction);
	void AcquireBus();
	void ReleaseBus();
};
