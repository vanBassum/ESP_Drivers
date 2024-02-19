#pragma once
#include "kernel.h"
#include "esp_base.h"
#include "spi/device.h"
#include "gpio.h"
#include "IGpio.h"
#include "gpiopin.h"


class ST7796S : public IDevice
{
    constexpr static const char* TAG = "ST7796S";
	Mutex mutex;

	//Dependencies:
    const char* spiDeviceKey = nullptr;
    std::shared_ptr<ISpiDevice> spiDevice;
    GpioPin dcPin;
    GpioPin rstPin;
    GpioPin blckPin;

    
    DeviceResult st7796s_send_cmd(uint8_t cmd);
    DeviceResult st7796s_send_data(const uint8_t *data, uint16_t length);
    DeviceResult st7796s_set_orientation(uint8_t orientation);
    DeviceResult st7796s_spi_transfer(const uint8_t* txData, uint8_t* rxData, size_t length);
    DeviceResult st7796s_send_color(const uint16_t* data, uint16_t length);

public:
	virtual DeviceResult DeviceSetConfig(IDeviceConfig& config) override;
    virtual DeviceResult DeviceLoadDependencies(std::shared_ptr<DeviceManager> deviceManager) override;
    virtual DeviceResult DeviceInit() override;

    DeviceResult DrawPixel(uint16_t x, uint16_t y, uint16_t color);
    DeviceResult SetWindow(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);
    DeviceResult WriteWindow(uint16_t* colors, size_t size);

    DeviceResult SetBacklight(bool value);

};

