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
    bool isrRegistered = false;
    std::function<void()> isrCallback;

    void DeviceIsrCallbackHandler()
    {
        if(isrCallback)
            isrCallback();
    }


public:

    Result DeviceSetConfig(IDeviceConfig& config, const char* propertyKey)
    {
        ContextLock lock(mutex);
        const char* temp = nullptr;
        RETURN_ON_ERR(config.getProperty(propertyKey, &temp));
        int result = sscanf(temp, "%m[^,],%hhu,%hhu", &deviceKey, &port, &pin);
        if(result != 3)
        {
            ESP_LOGE(TAG, "Error parsing %s", propertyKey);
            return Result::Error;
        }
        return Result::Ok;
    }

    Result DeviceLoadDependencies(std::shared_ptr<DeviceManager> deviceManager)
    {
        ContextLock lock(mutex);
        RETURN_ON_ERR(deviceManager->getDeviceByKey<IGpio>(deviceKey, device));
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
        uint8_t val = 0;
        RETURN_ON_ERR(device->GpioRead(port, 1<<pin, &val));
        *value = val?true:false;
        return Result::Ok;
    }

    Result GpioConfigure(const GpioConfig* config)
    {
        ContextLock lock(mutex);
        return device->GpioConfigure(port, 1<<pin, config);
    }

    Result RegisterISR(std::function<void()> callback)
    {
        ContextLock lock(mutex);
        if(isrCallback)
            ESP_LOGW(TAG, "Overwriting ISR callback for pin %d.%d", (int)port, (int)pin);
        isrCallback = callback;
        if(!isrRegistered)
        {
            RETURN_ON_ERR(device->GpioIsrAddCallback(port, pin, [&](){DeviceIsrCallbackHandler();}));
            isrRegistered = true;
        }
        return Result::Ok;
    }

    Result UnRegisterISR()
    {
        ContextLock lock(mutex);
        isrCallback = NULL;
        if(isrRegistered)
        {
            RETURN_ON_ERR(device->GpioIsrRemoveCallback(port, pin));
            isrRegistered = false;
        }
        return Result::Ok;
    }
};
