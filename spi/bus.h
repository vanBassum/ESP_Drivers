#pragma once
#include "IDevice.h"


class SpiBus : public IDevice {
    
    constexpr static const char* TAG = "SpiBus";
public:
    virtual ~SpiBus() {}

    virtual ErrCode setConfig(IDeviceConfig& config) override
    {
        DEV_RETURN_ON_FALSE(checkStatus(Status::Created), ErrCode::WrongStatus, TAG, "Tried to config driver with status %d", (int)getStatus());

        // Read the config, and set the status!
        setStatus(Status::Configured);
        return ErrCode::Ok;
    }

    virtual ErrCode init(std::shared_ptr<DeviceManager> deviceManager) override
    {
        DEV_SET_STATUS_AND_RETURN_ON_FALSE(deviceManager, Status::Error, ErrCode::NullPtr, TAG, "Cannot initialize device, devicemanger nullptr");
        DEV_SET_STATUS_AND_RETURN_ON_FALSE(checkStatus(Status::Configured), Status::Error, ErrCode::WrongStatus, TAG, "Tried to initialize driver with status %d", (int)getStatus());

        // Use the devicemanager to get all dependencies.   (There are no dependecies in this case)

        // Check for dependencies to be status ready.       (There are no dependecies in this case)

        // TODO Initialize the drivers
        setStatus(Status::Ready);
        return ErrCode::Ok;
    }
};


//#include "driver/spi_master.h"
//#include "driver/gpio.h"
//#include <vector>
//#include <functional>
//class SPIDevice;
//class SPIBus
//{
//	static constexpr const char* TAG = "SPIBus";
//
//public:
//    struct Config
//    {
//		spi_host_device_t host = SPI_HOST_MAX;
//		int dmaChannel = -1;
//		spi_bus_config_t config = {};
//		
//    };
//
//    SPIBus() = default;
//    ~SPIBus() = default;
//    void setConfig(const Config &newConfig);
//    void init();
//    bool isInitialized() const;
//
//private:
//    Config config = {}; // Default initialize Config
//    bool initialized = false;
//	friend SPIDevice;
//};
