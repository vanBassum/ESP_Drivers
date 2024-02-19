#pragma once
#include "ISpiDevice.h"
#include "bus.h"
#include "DeviceManager.h"
#include "IGpio.h"

#define SPI_CS_PIN_A0 						GPIO_NUM_5
#define SPI_CS_PIN_A1 						GPIO_NUM_32
#define SPI_CS_PIN_A2 						GPIO_NUM_2



class SpiDevice : public ISpiDevice {
    Mutex mutex;
    constexpr static const char* TAG = "SpiDevice";

    //Error on the bus
    const char* spiBusKey = nullptr;
    std::shared_ptr<SpiBus> spiBus;

    //Error and settings on the custom CS
    uint8_t csPort = 0;
    uint8_t csPin = 0;
    const char* csDeviceKey = nullptr;
    std::shared_ptr<IGpio> csDevice;

    //Config and handle for device
    spi_device_handle_t handle = NULL;
    spi_device_interface_config_t devConfig = {};

    //Callbacks for custom chip select
    static void IRAM_ATTR Select(spi_transaction_t* t);
    static void IRAM_ATTR Deselect(spi_transaction_t* t);

public:
    
    virtual ~SpiDevice() {}
    virtual DeviceResult DeviceSetConfig(IDeviceConfig& config) override ;
    virtual DeviceResult DeviceLoadDependencies(std::shared_ptr<DeviceManager> deviceManager) override;
    virtual DeviceResult DeviceInit() override;
    virtual DeviceResult SpiTransmit(const uint8_t* txData, uint8_t* rxData, size_t size, SPIFlags flags) override;
    DeviceResult Transmit(spi_transaction_t* transaction, SPIFlags flags);
    
};

