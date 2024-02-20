#pragma once
#include "interfaces.h"
#include "DeviceManager.h"
#include "kernel.h"
#include "MAX14830.h"
#include <functional>
#include <list>

class MAX14830Uart : public IUART
{
    constexpr static const char *TAG = "MAX14830Uart";
    Mutex mutex;

    // Dependencies:
    const char *maxDeviceKey = nullptr;
    std::shared_ptr<MAX14830> maxDevice;

    // Config:
    uint8_t port = 0;

    // Methods:
	DeviceResult max310x_set_baud(uint32_t baud);

public:
    virtual DeviceResult DeviceSetConfig(IDeviceConfig &config) override;
    virtual DeviceResult DeviceLoadDependencies(std::shared_ptr<DeviceManager> deviceManager) override;
    virtual DeviceResult DeviceInit() override;

    virtual DeviceResult UartConfigure(const UartConfig* config) override;
    virtual DeviceResult StreamWrite(const uint8_t* data, size_t length, size_t* written = nullptr, TickType_t timeout = portMAX_DELAY ) override;
    virtual DeviceResult StreamRead(uint8_t* data, size_t length, size_t* read = nullptr, TickType_t timeout = portMAX_DELAY) override;


};
