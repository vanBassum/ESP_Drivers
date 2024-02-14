#pragma once
#include "kernel.h"
#include "esp_base.h"
#include "spi/device.h"
#include "gpio.h"
#include "IDevice.h"

//https://github.com/RobTillaart/MCP23S17/blob/master/MCP23S17.cpp
//https://www.esp32.com/viewtopic.php?t=9309

class MCP23S17 : public IDevice
{
    constexpr static const char* TAG = "MCP23S17";
	Mutex mutex;
    const char* spiDeviceKey = nullptr;
    std::shared_ptr<SpiDevice> spiDevice;	//TODO, MAKE USE OF INTERFACE!!!!

	uint8_t devAddr = 0;	//I dont think this is used in SPI
	uint8_t pinBuffer[2] = {0};
	uint8_t pinDirBuffer[2] = {0};
	
	ErrCode Transmit(uint8_t * txData, uint8_t * rxData, uint8_t count);
	ErrCode Read8(uint8_t reg, uint8_t* value);
	ErrCode Write8(uint8_t reg, uint8_t value);
	//ErrCode Read16(uint8_t reg, uint16_t* value);
	//ErrCode Write16(uint8_t reg, uint16_t value);

public:
	virtual ErrCode setConfig(IDeviceConfig& config) override;
    virtual ErrCode loadDependencies(std::shared_ptr<DeviceManager> deviceManager) override;
    virtual ErrCode init() override;

	//ErrCode SetPinsMode(Pins mask, PinModes mode);
	//ErrCode SetPins(Pins mask, Pins value);
	//ErrCode GetPins(Pins mask);
	//ErrCode ConsecutivePinWriting(Pins mask, Pins* values, size_t size);
};

