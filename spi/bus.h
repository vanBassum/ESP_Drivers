#pragma once
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include <vector>
#include <functional>
class SPIDevice;
class SPIBus
{
	static constexpr const char* TAG = "SPIBus";

public:
    struct Config
    {
		spi_host_device_t host = SPI_HOST_MAX;
		int dmaChannel = -1;
		spi_bus_config_t config = {};
		
    };

private:
    Config config = {}; // Default initialize Config
    bool initialized = false;
	friend SPIDevice;
public:
    SPIBus() = default;
    ~SPIBus() = default;
    void setConfig(const Config &newConfig);
    void init();
    bool isInitialized() const;
};
