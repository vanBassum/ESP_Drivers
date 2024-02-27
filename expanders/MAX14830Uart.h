#pragma once
#include "interfaces.h"
#include "devicemanagement.h"
#include "rtos.h"
#include "MAX14830.h"
#include <functional>
#include <list>

class MAX14830Uart : public IUART
{
    constexpr static const char *TAG = "MAX14830Uart";
    Mutex mutex;

    // Config:
    uint8_t port = 0;
    UartConfig initialConfig;
    const char* maxDeviceKey;

    // Dependencies:
    std::shared_ptr<MAX14830> maxDevice;

    // Methods:
	Result max310x_set_baud(uint32_t baud);

public:
    virtual Result DeviceSetConfig(IDeviceConfig &config) override;
    virtual Result DeviceLoadDependencies(std::shared_ptr<DeviceManager> deviceManager) override;
    virtual Result DeviceInit() override;

    virtual Result UartConfigure(const UartConfig* config) override;
    virtual Result StreamWrite(const uint8_t* data, size_t length, size_t* written = nullptr, TickType_t timeout = portMAX_DELAY ) override;
    virtual Result StreamRead(uint8_t* data, size_t length, size_t* read = nullptr, TickType_t timeout = portMAX_DELAY) override;


};
