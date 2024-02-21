#pragma once
#include "helpers.h"
#include "deviceConfig.h"


enum class Result : uint32_t
{
    Ok = 0,                                             // No error
    Error = 1,                                          // Something was null
    NotSupported = 2,
};


using DeviceResult [[deprecated]] = Result;


enum class DeviceStatus : uint32_t
{
    Ready = 0,                                          // Device is initialized and ready to use.
    Configuring = 1,                                    // Device was created, waiting for configuration.
    Dependencies = 2,                                   // Device was configured, waiting for dependencies.
    Initializing = 3,                                   // Device needs initialisation.
    ConfigError = 4,                                    // Device couldn't be created because of a configuration error.
    Error = 5,                                          // Device is end of live, all references should be removed.
};


class DeviceManager;
class IDevice {
    constexpr static const char* TAG = "IDevice";
    DeviceStatus status = DeviceStatus::Configuring;
    constexpr static const char* StatusStr[] = {
        "Ready",         
        "Created",    
        "Dependencies",    
        "Initializing",           
        "ConfigError",     
        "Error",           
    };

public:
    const char* key;

protected: 
    void DeviceSetStatus(DeviceStatus newStatus) 
    {
        if(newStatus == DeviceStatus::Ready || status == DeviceStatus::Ready)  //Only show transistions from and to Ready
            ESP_LOGI(TAG, "'%s' is '%s'", key, StatusStr[(int)newStatus]);


        status = newStatus;
    }

public:
    virtual ~IDevice() {}
    virtual DeviceResult DeviceSetConfig(IDeviceConfig& config) = 0;
    virtual DeviceResult DeviceLoadDependencies(std::shared_ptr<DeviceManager> deviceManager) = 0;
    virtual DeviceResult DeviceInit() = 0;
    DeviceStatus DeviceGetStatus() {return status;}
    bool DeviceCheckStatus(DeviceStatus cStatus) { return cStatus == status;}
};
