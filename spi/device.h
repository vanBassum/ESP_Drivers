#pragma once
#include "ISpiDevice.h"
#include "bus.h"
#include "DeviceManager.h"




class SpiDevice : public ISpiDevice {
    
    constexpr static const char* TAG = "SpiDevice";
    const char* spiBusKey = nullptr;
    std::shared_ptr<SpiBus> spiBus;
    spi_device_handle_t handle = NULL;
    spi_device_interface_config_t devConfig = {};

public:
    virtual ~SpiDevice() {}

    //Since this is handeled by the devicemanager, assume this is only called on apropiate times. So no need to check the status of the driver.
    virtual ErrCode setConfig(IDeviceConfig& config) override  
    {
        DEV_SET_STATUS_AND_RETURN_ON_FALSE(config.getProperty("spiBus", &spiBusKey), Status::ConfigError, ErrCode::ConfigError, TAG, "No property found for 'spiBus'");
        config.getProperty("command_bits", &devConfig.command_bits);
        config.getProperty("address_bits", &devConfig.address_bits);
        config.getProperty("dummy_bits", &devConfig.dummy_bits);
        config.getProperty("mode", &devConfig.mode);
        config.getProperty("duty_cycle_pos", &devConfig.duty_cycle_pos);
        config.getProperty("cs_ena_pretrans", &devConfig.cs_ena_pretrans);
        config.getProperty("cs_ena_posttrans", &devConfig.cs_ena_posttrans);
        config.getProperty("clock_speed_hz", &devConfig.clock_speed_hz);
        config.getProperty("input_delay_ns", &devConfig.input_delay_ns);
        config.getProperty("spics_io_num", &devConfig.spics_io_num);
        config.getProperty("flags", &devConfig.flags);
        config.getProperty("queue_size", &devConfig.queue_size);
        //config.getProperty("pre_cb", &devConfig.pre_cb);
        //config.getProperty("post_cb", &devConfig.post_cb);



        
        setStatus(Status::Dependencies);
        return ErrCode::Ok;
    }

    //Since this is handeled by the devicemanager, assume this is only called on apropiate times. So no need to check the status of the driver. Also assume the devicemanger is not null.
    virtual ErrCode loadDependencies(std::shared_ptr<DeviceManager> deviceManager) override
    {
        GET_DEV_OR_RETURN(spiBus, deviceManager->getDeviceByKey<SpiBus>(spiBusKey), Status::Dependencies, ErrCode::Dependency, TAG, "Dependencies not ready %d", (int)getStatus());
        setStatus(Status::Initializing);
        return ErrCode::Ok;
    }

    //Since this is handeled by the devicemanager, assume this is only called on apropiate times. So no need to check the status of the driver.
    virtual ErrCode init() override
    {
        spi_host_device_t host;
        DEV_SET_STATUS_AND_RETURN_ON_FALSE(spiBus->GetHost(&host) == ErrCode::Ok, Status::Dependencies, ErrCode::Dependency, TAG, "spiBus->GetHost error");

        if(spi_bus_add_device(host, &devConfig, &handle) != ESP_OK)
        {
            setStatus(Status::Error);
            return ErrCode::InitFault;
        }

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
