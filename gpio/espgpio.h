#pragma once
#include "interfaces.h"
#include "DeviceManager.h"
#include <unordered_map>
#include <functional>
#include <list>
#include "driver/gpio.h"

class EspGpio : public IGpio // Prefer to use interface, otherwise use IDevice
{
    constexpr static const char *TAG = "ESPGpio";
    Mutex mutex;
    void triggerCallbacks(uint32_t pin, uint8_t changedPins);
    static void gpio_isr_handler(void *arg);

    struct IsrHandle
    {
        EspGpio* device;
        gpio_num_t pin;
        std::function<void()> callback;
    };

    static std::list<std::shared_ptr<IsrHandle>> callbacks;

public:
    virtual DeviceResult DeviceSetConfig(IDeviceConfig &config) override;
    virtual DeviceResult DeviceLoadDependencies(std::shared_ptr<DeviceManager> deviceManager) override;
    virtual DeviceResult DeviceInit() override;

    virtual DeviceResult GpioConfigure(uint32_t port, uint8_t mask, const GpioConfig* config)  override;
    virtual DeviceResult GpioRead(uint32_t port, uint8_t mask, uint8_t *value)  override;
    virtual DeviceResult GpioWrite(uint32_t port, uint8_t mask, uint8_t value) override;
    virtual DeviceResult GpioIsrAddCallback(uint32_t port, uint8_t pin, std::function<void()> callback) override;
    virtual DeviceResult GpioIsrRemoveCallback(uint32_t port, uint8_t pin)  override;
};

