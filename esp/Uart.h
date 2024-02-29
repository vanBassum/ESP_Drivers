#pragma once
#include "rtos.h"
#include "esp_base.h"
#include "interfaces/IUART.h"
#include "driver/uart.h"
#include "driver/gpio.h"


class ESPUart : public IUART
{
    constexpr static const char* TAG = "ESPUart";
	Mutex mutex;

	//Config:
    uart_port_t portNo = 0;
    gpio_num_t txPin = GPIO_NUM_NC;
    gpio_num_t rxPin = GPIO_NUM_NC;
    uint32_t bufferSize = 32;

    std::unique_ptr<uint8_t[]> rxBuffer;
public:
	virtual Result DeviceSetConfig(IDeviceConfig& config) override;
    virtual Result DeviceLoadDependencies(std::shared_ptr<DeviceManager> deviceManager) override;
    virtual Result DeviceInit() override;

    virtual Result UartConfigure(const UartConfig* config) override;
    virtual Result StreamWrite(const uint8_t* data, size_t length, size_t* written = nullptr, TickType_t timeout = portMAX_DELAY ) override;
    virtual Result StreamRead(uint8_t* data, size_t length, size_t* read = nullptr, TickType_t timeout = portMAX_DELAY) override;
};

