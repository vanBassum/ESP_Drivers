#pragma once
#include "helpers.h"
#include "deviceConfig.h"


class DeviceManager;
class IDevice {
protected:
    constexpr static const char* TAG = "IDevice";
public:
    const char* key;

    enum class Status : uint32_t
    {
        Created = 0,                                        // Device was created, waiting for configuration.
        Dependencies = 1,                                   // Device was configured, waiting for dependencies.
        Initializing = 2,                                   // Device needs initialisation.
        Ready = 3,                                          // Device is initialized and ready to use.
        ConfigError = 4,                                    // Device couldn't be created because of a configuration error.
        Error = 5,                                          // Device is end of live, all references should be removed.
    };

    constexpr static const char* StatusStr[] = {
        "Created",         
        "Dependencies",    
        "Initializing",    
        "Ready",           
        "ConfigError",     
        "Error",           
    };


    enum class ErrCode : uint32_t
    {
        Ok = 0,                                             // No error
        NullPtr = 1,                                        // Something was null
        WrongStatus = 2,                                    // Device is in wrong state
        Dependency = 3,                                     // Some dependency isn't available
        ConfigError = 4,                                    // Configuration problem
        InitFault   = 5,
    };

private:
    Status status = Status::Created;

protected: 
    void setStatus(Status newStatus) 
    {
        if(newStatus == Status::Ready || status == Status::Ready)  //Only show transistions from and to Ready
            ESP_LOGI(TAG, "'%s' status changed. '%s' to '%s'", key, StatusStr[(int)status], StatusStr[(int)newStatus]);

        status = newStatus;
    }

public:
    virtual ~IDevice() {}
    virtual ErrCode setConfig(IDeviceConfig& config) = 0;
    virtual ErrCode loadDependencies(std::shared_ptr<DeviceManager> deviceManager) = 0;
    virtual ErrCode init() = 0;
    Status getStatus() {return status;}
    bool checkStatus(Status cStatus) { return cStatus == status;}
};
