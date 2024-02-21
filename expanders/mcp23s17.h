#pragma once
#include "rtos.h"
#include "esp_base.h"
#include "spi/device.h"
#include "gpio.h"
#include "IGpio.h"

//https://github.com/RobTillaart/MCP23S17/blob/master/MCP23S17.cpp
//https://www.esp32.com/viewtopic.php?t=9309

class MCP23S17 : public IGpio
{
    constexpr static const char* TAG = "MCP23S17";
	Mutex mutex;

	//Config:
    const char* spiDeviceKey = nullptr;
    std::shared_ptr<ISpiDevice> spiDevice;
	uint8_t devAddr = 0;	//I dont think this is used in the SPI version of this chip.
	uint8_t pinBuffer[2] = {0};
	uint8_t pinDirBuffer[2] = {0};
	
	Result Transmit(uint8_t * txData, uint8_t * rxData, uint8_t count);
	Result Read8(uint8_t reg, uint8_t* value);
	Result Write8(uint8_t reg, uint8_t value);
	Result Read16(uint8_t reg, uint16_t* value);
	Result Write16(uint8_t reg, uint16_t value);

public:
	virtual Result DeviceSetConfig(IDeviceConfig& config) override;
    virtual Result DeviceLoadDependencies(std::shared_ptr<DeviceManager> deviceManager) override;
    virtual Result DeviceInit() override;

    virtual Result GpioConfigure(uint32_t port, uint8_t mask, const GpioConfig* config)  override;
    virtual Result GpioRead(uint32_t port, uint8_t mask, uint8_t *value)  override;
    virtual Result GpioWrite(uint32_t port, uint8_t mask, uint8_t value)  override;

};

