#pragma once
#include "ISpiDevice.h"
#include "bus.h"
#include "DeviceManager.h"




class SpiDevice : public ISpiDevice {
    
    constexpr static const char* TAG = "SpiDevice";
    const char* spiBusKey = nullptr;
    std::shared_ptr<SpiBus> bus;

public:
    virtual ~SpiDevice() {}

    //Since this is handeled by the devicemanager, assume this is only called on apropiate times. So no need to check the status of the driver.
    virtual ErrCode setConfig(IDeviceConfig& config) override  
    {
        GET_STR_OR_RETURN(spiBusKey, config.getProperty("spiBus"), Status::ConfigError, ErrCode::ConfigError, TAG, "No property found for 'spiBus'");
        setStatus(Status::Dependencies);
        return ErrCode::Ok;
    }

    //Since this is handeled by the devicemanager, assume this is only called on apropiate times. So no need to check the status of the driver. Also assume the devicemanger is not null.
    virtual ErrCode loadDependencies(std::shared_ptr<DeviceManager> deviceManager) override
    {
        GET_DEV_OR_RETURN(bus, deviceManager->getDeviceByKey<SpiBus>(spiBusKey), Status::Dependencies, ErrCode::Dependency, TAG, "Dependencies not ready %d", (int)getStatus());
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

    virtual ErrCode Write(uint8_t* data, size_t size) override 
    {
        //Ensure the device is in the ready state
        DEV_RETURN_ON_FALSE(checkStatus(Status::Ready), ErrCode::WrongStatus, TAG, "Driver not ready, status %d", (int)getStatus());

        //TODO: Implement writing of data. Dont forget to check the result of the functions you call from the dependecies.
        return ErrCode::Ok;
    }

    virtual ErrCode Read(uint8_t* data, size_t size) override 
    {
        //Ensure the device is in the ready state
        DEV_RETURN_ON_FALSE(checkStatus(Status::Ready), ErrCode::WrongStatus, TAG, "Driver not ready, status %d", (int)getStatus());

        //TODO: Implement reading of data. Dont forget to check the result of the functions you call from the dependecies.
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
