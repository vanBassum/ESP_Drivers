#pragma once
#include "helpers.h"
#include "DeviceManager.h"
#include "IGpio.h"
#include "IDevice.h"


class GpioPin
{
    const char* TAG = "GpioPin";
    uint8_t     pin;
    uint8_t     port;
    const char* deviceKey = nullptr;
    std::shared_ptr<IGpio> device;
    Mutex mutex;
public:

    Result DeviceSetConfig(IDeviceConfig& config, const char* propertyKey)
    {
        ContextLock lock(mutex);
        const char* temp = nullptr;
        RETURN_ON_ERR(config.getProperty(propertyKey, &temp));
        DEV_RETURN_ON_FALSE(sscanf(temp, "%m[^,],%hhu,%hhu", &deviceKey, &port, &pin) == 3,  Result::Error,  TAG, "Error parsing %s", propertyKey);
        return Result::Ok;
    }

    Result DeviceLoadDependencies(std::shared_ptr<DeviceManager> deviceManager)
    {
        ContextLock lock(mutex);
        device = deviceManager->getDeviceByKey<IGpio>(deviceKey);
        if(device == nullptr)
        {
            ESP_LOGE(TAG, "Dependencies not ready %s", deviceKey);
            return Result::Error;
        }
        return Result::Ok;
    }

    Result DeviceInit()
    {
        ContextLock lock(mutex);
        return Result::Ok;
    }

    Result GpioPinWrite(bool value)
    {
        ContextLock lock(mutex);
        return device->GpioWrite(port, 1<<pin, value?0xFF:0x00);
    }

    Result GpioPinRead(bool* value)
    {
        ContextLock lock(mutex);
        uint8_t val;
        DEV_RETURN_ON_ERROR_SILENT(device->GpioRead(port, 1<<pin, &val));
        *value = val?true:false;
        return Result::Ok;
    }

    Result GpioConfigure(const GpioConfig* config)
    {
        ContextLock lock(mutex);
        return device->GpioConfigure(port, 1<<pin, config);
    }
};
