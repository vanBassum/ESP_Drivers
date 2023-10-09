#pragma once
#include "driver/spi_master.h"
#include "mutex.h"

class SPIBus;

class SPIDevice
{
	Mutex mutex;
	spi_device_handle_t spi = NULL;
public:
	SPIDevice(SPIBus& bus, const spi_device_interface_config_t& devConfig);
	void transfer(uint8_t* txData, uint8_t* rxData, size_t length);
	void PollingTransmit(spi_transaction_t* transaction);
	void Transmit(spi_transaction_t* transaction);
	void AcquireBus();
	void ReleaseBus();
};
