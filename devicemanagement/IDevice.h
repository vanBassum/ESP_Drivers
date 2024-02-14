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
        Configured = 1,                                     // Device was configured, waiting for initialisaion.
        Ready = 2,                                          // Device is initialized and ready to use.
        ConfigError = 3,                                    // Device couldn't be created because of a configuration error.
        Error = 4,                                          // Device is end of live, all references should be removed.
    };


    enum class ErrCode : uint32_t
    {
        Ok = 0,                                             // No error
        NullPtr = 1,                                        // Something was null
        WrongStatus = 2,                                    // Device is in wrong state
        Dependency = 3,                                     // Some dependency isn't available
        ConfigError = 4,                                    // Configuration problem
    };

private:
    Status status = Status::Created;

protected: 
    void setStatus(Status newStatus) 
    {
        ESP_LOGI(TAG, "Device '%s' status changed. '%d' to '%d'", key, (int)status, (int)newStatus);
        status = newStatus;
    }

public:
    virtual ~IDevice() {}
    virtual ErrCode setConfig(IDeviceConfig& config) = 0;
    virtual ErrCode init(std::shared_ptr<DeviceManager> deviceManager) = 0;
    Status getStatus() {return status;}
    bool checkStatus(Status cStatus) { return cStatus == status;}
};
