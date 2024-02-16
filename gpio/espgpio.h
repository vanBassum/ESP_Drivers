#pragma once
#include "interfaces.h"
#include "DeviceManager.h"
#include <unordered_map>
#include <functional>
#include <vector>

class EspGpio : public IGpio // Prefer to use interface, otherwise use IDevice
{
    constexpr static const char *TAG = "ESPGpio";
    Mutex mutex;
    std::unordered_map<uint32_t, std::unordered_map<uint8_t, std::function<void()>>> isrCallbacks;
    gpio_isr_handle_t isr_handle;

    void triggerCallbacks(uint32_t port, uint8_t changedPins);
    static void gpio_isr_handler(void *arg);

public:
    virtual DeviceResult setDeviceConfig(IDeviceConfig &config) override;
    virtual DeviceResult loadDeviceDependencies(std::shared_ptr<DeviceManager> deviceManager) override;
    virtual DeviceResult init() override;

    virtual DeviceResult portConfigure(uint32_t port, uint8_t mask, GpioConfig *config) override;
    virtual DeviceResult portRead(uint32_t port, uint8_t mask, uint8_t *value) override;
    virtual DeviceResult portWrite(uint32_t port, uint8_t mask, uint8_t value) override;

    virtual DeviceResult portIsrAddCallback(uint32_t port, uint8_t pin, std::function<void()> callback) override;
    virtual DeviceResult portIsrRemoveCallback(uint32_t port, uint8_t pin) override;
};

