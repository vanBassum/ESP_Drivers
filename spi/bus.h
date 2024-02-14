#pragma once
#include "IDevice.h"


class SpiBus : public IDevice {
    
    constexpr static const char* TAG = "SpiBus";
public:
    virtual ~SpiBus() {}

    //Since this is handeled by the devicemanager, assume this is only called on apropiate times. So no need to check the status of the driver.
    virtual ErrCode setConfig(IDeviceConfig& config) override  
    {
        //GET_STR_OR_RETURN(spiBusKey, config.getProperty("spiBus"), Status::ConfigError, ErrCode::ConfigError, TAG, "No property found for 'spiBus'");
        setStatus(Status::Dependencies);
        return ErrCode::Ok;
    }

    //Since this is handeled by the devicemanager, assume this is only called on apropiate times. So no need to check the status of the driver. Also assume the devicemanger is not null.
    virtual ErrCode loadDependencies(std::shared_ptr<DeviceManager> deviceManager) override
    {
       // GET_DEV_OR_RETURN(bus, deviceManager->getDeviceByKey<SpiBus>(spiBusKey), Status::Dependencies, ErrCode::Dependency, TAG, "Dependencies not ready %d", (int)getStatus());
        setStatus(Status::Initializing);
        return ErrCode::Ok;
    }

    //Since this is handeled by the devicemanager, assume this is only called on apropiate times. So no need to check the status of the driver.
    virtual ErrCode init() override
    {
        //TODO: Initialize the driver here! Dont forget to check the result of the functions you call from the dependecies.
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
