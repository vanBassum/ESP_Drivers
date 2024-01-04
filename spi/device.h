#pragma once
#include "mutex.h"
#include <memory>
#include <functional>
#include "bus.h"

class SPIDevice {
    static constexpr const char* TAG = "SPIDevice";
    Mutex mutex;
    spi_device_handle_t handle = NULL;
    std::shared_ptr<SPIBus> spiBus;
    bool initialized = false;

public:
    struct Config {
        spi_device_interface_config_t devConfig = {};
    };

private:
	Config config = {};

public:
    SPIDevice(std::shared_ptr<SPIBus> spiBus);
    ~SPIDevice() = default;

    void setConfig(const Config& newConfig);
    void init();
    void transfer(uint8_t* txData, uint8_t* rxData, size_t length);
    void PollingTransmit(spi_transaction_t* transaction);
    void Transmit(spi_transaction_t* transaction);
    void AcquireBus();
    void ReleaseBus();
	bool isInitialized() const;
};
