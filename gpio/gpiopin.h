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

    DeviceResult DeviceSetConfig(IDeviceConfig& config, const char* propertyKey)
    {
        ContextLock lock(mutex);
        const char* temp = nullptr;
        DEV_RETURN_ON_FALSE(config.getProperty(propertyKey, &temp), DeviceResult::Error,  TAG, "Missing parameter: %s", propertyKey);
        DEV_RETURN_ON_FALSE(sscanf(temp, "%m[^,],%hhu,%hhu", &deviceKey, &port, &pin) == 3,  DeviceResult::Error,  TAG, "Error parsing %s", propertyKey);
        return DeviceResult::Ok;
    }

    DeviceResult DeviceLoadDependencies(std::shared_ptr<DeviceManager> deviceManager)
    {
        ContextLock lock(mutex);
        device = deviceManager->getDeviceByKey<IGpio>(deviceKey);
        if(device == nullptr)
        {
            ESP_LOGE(TAG, "Dependencies not ready %s", deviceKey);
            return DeviceResult::Error;
        }
        return DeviceResult::Ok;
    }

    DeviceResult DeviceInit()
    {
        ContextLock lock(mutex);
        return DeviceResult::Ok;
    }

    DeviceResult GpioPinWrite(bool value)
    {
        ContextLock lock(mutex);
        return device->GpioWrite(port, 1<<pin, value?0xFF:0x00);
    }

    DeviceResult GpioPinRead(bool* value)
    {
        ContextLock lock(mutex);
        uint8_t val;
        DEV_RETURN_ON_ERROR_SILENT(device->GpioRead(port, 1<<pin, &val));
        *value = val?true:false;
        return DeviceResult::Ok;
    }

    DeviceResult GpioConfigure(const GpioConfig* config)
    {
        ContextLock lock(mutex);
        return device->GpioConfigure(port, 1<<pin, config);
    }
};
