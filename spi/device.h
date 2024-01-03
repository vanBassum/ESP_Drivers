#pragma once
#include "mutex.h"
#include <memory>
#include <functional>
#include "bus.h"

class SPIDevice
{
	constexpr const static char* TAG = "SPIDevice";
	Mutex mutex;
	spi_device_handle_t handle = NULL;
	std::shared_ptr<SPIBus> spiBus;
public:
	spi_device_interface_config_t devConfig;

	SPIDevice(std::shared_ptr<SPIBus> spiBus, std::function<void(SPIDevice& dev)> configurator);
	void transfer(uint8_t* txData, uint8_t* rxData, size_t length);
	void PollingTransmit(spi_transaction_t* transaction);
	void Transmit(spi_transaction_t* transaction);
	void AcquireBus();
	void ReleaseBus();
};




