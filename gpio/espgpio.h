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
    virtual Result DeviceSetConfig(IDeviceConfig &config) override;
    virtual Result DeviceLoadDependencies(std::shared_ptr<DeviceManager> deviceManager) override;
    virtual Result DeviceInit() override;

    virtual Result GpioConfigure(uint32_t port, uint8_t mask, const GpioConfig* config)  override;
    virtual Result GpioRead(uint32_t port, uint8_t mask, uint8_t *value)  override;
    virtual Result GpioWrite(uint32_t port, uint8_t mask, uint8_t value) override;
    virtual Result GpioIsrAddCallback(uint32_t port, uint8_t pin, std::function<void()> callback) override;
    virtual Result GpioIsrRemoveCallback(uint32_t port, uint8_t pin)  override;
};

