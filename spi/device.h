#pragma once
#include "ISpiDevice.h"
#include "bus.h"
#include "DeviceManager.h"


class SpiDevice : public IDevice {
    
    constexpr static const char* TAG = "SpiDevice";
    const char* spiBusKey = nullptr;
    std::shared_ptr<SpiBus> bus;

public:
    virtual ~SpiDevice() {}

    virtual ErrCode setConfig(IDeviceConfig& config) override
    {
        DEV_RETURN_ON_FALSE(checkStatus(Status::Created), ErrCode::WrongStatus, TAG, "Tried to config driver with status %d", (int)getStatus());

        // Read the config
        GET_STR_OR_RETURN(spiBusKey, config.getProperty("spiBus"), TAG, "Couln't get property 'spiBus'");

        setStatus(Status::Configured);
        return ErrCode::Ok;
    }

    virtual ErrCode init(std::shared_ptr<DeviceManager> deviceManager) override
    {
        DEV_SET_STATUS_AND_RETURN_ON_FALSE(deviceManager, Status::Error, ErrCode::NullPtr, TAG, "Cannot initialize device, devicemanger nullptr");
        DEV_SET_STATUS_AND_RETURN_ON_FALSE(checkStatus(Status::Configured), Status::Error, ErrCode::WrongStatus, TAG, "Tried to initialize driver with status %d", (int)getStatus());

        // Use the devicemanager to get all dependencies.
        bus = deviceManager->getDeviceByKey<SpiBus>(spiBusKey);
        DEV_RETURN_ON_FALSE(bus, ErrCode::Dependency, TAG, "Cannot initialize device, dependency 'SpiBus' missing");

        // Check for dependencies to be status ready.       (There are no dependecies in this case)
        DEV_RETURN_ON_FALSE(bus->checkStatus(Status::Ready), ErrCode::Dependency, TAG, "Cannot initialize device, dependency 'SpiBus' not ready");

        // TODO: Initialize the drivers


        setStatus(Status::Ready);
        return ErrCode::Ok;
    }
};




//#include "mutex.h"
//#include <memory>
//#include <functional>
//#include "bus.h"
//
//class SPIDevice {
//    static constexpr const char* TAG = "SPIDevice";
//    Mutex mutex;
//    spi_device_handle_t handle = NULL;
//    std::shared_ptr<SPIBus> spiBus;
//    bool initialized = false;
//
//public:
//    struct Config {
//        spi_device_interface_config_t devConfig = {};
//    };
//
//private:
//	Config config = {};
//
//public:
//    SPIDevice(std::shared_ptr<SPIBus> spiBus);
//    ~SPIDevice() = default;
//
//    void setConfig(const Config& newConfig);
//    void init();
//    void transfer(uint8_t* txData, uint8_t* rxData, size_t length);
//    void PollingTransmit(spi_transaction_t* transaction);
//    void Transmit(spi_transaction_t* transaction);
//    void AcquireBus();
//    void ReleaseBus();
//	bool isInitialized() const;
//};
