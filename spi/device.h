#pragma once
#include "ISpiDevice.h"
#include "bus.h"
#include "DeviceManager.h"

#define SPI_CS_PIN_A0 						GPIO_NUM_5
#define SPI_CS_PIN_A1 						GPIO_NUM_32
#define SPI_CS_PIN_A2 						GPIO_NUM_2


//TODO: use gpio device to select cs pin!!!
void IRAM_ATTR Select(spi_transaction_t* t);
void IRAM_ATTR Deselect(spi_transaction_t* t);

class SpiDevice : public ISpiDevice {
    Mutex mutex;
    constexpr static const char* TAG = "SpiDevice";
    const char* spiBusKey = nullptr;
    std::shared_ptr<SpiBus> spiBus;
    spi_device_handle_t handle = NULL;
    spi_device_interface_config_t devConfig = {};

public:
    uint8_t customCsPin = 0;
    virtual ~SpiDevice() {}
    virtual DeviceResult setDeviceConfig(IDeviceConfig& config) override ;
    virtual DeviceResult loadDeviceDependencies(std::shared_ptr<DeviceManager> deviceManager) override;
    virtual DeviceResult init() override;
    //virtual ErrCode Write(uint8_t* data, size_t size) override ;
    //virtual ErrCode Read(uint8_t* data, size_t size) override ;
    virtual DeviceResult Transmit(uint8_t* txData, uint8_t* rxData, size_t size) override;
    
};

