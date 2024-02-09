#pragma once
#include "kernel.h"
#include "esp_base.h"
#include "spi/device.h"
#include "gpio.h"

//https://github.com/RobTillaart/MCP23S17/blob/master/MCP23S17.cpp
//https://www.esp32.com/viewtopic.php?t=9309

class MCP23S17
{
public:
	enum class Pins
	{
		NONE = 0x0000,
		A0   = 0x0001,
		A1   = 0x0002,
		A2   = 0x0004,
		A3   = 0x0008,
		A4   = 0x0010,
		A5   = 0x0020,
		A6   = 0x0040,
		A7   = 0x0080,
		B0   = 0x0100,
		B1   = 0x0200,
		B2   = 0x0400,
		B3   = 0x0800,
		B4   = 0x1000,
		B5   = 0x2000,
		B6   = 0x4000,
		B7   = 0x8000,
		ALL  = 0xFFFF,
	};

private:
	const char* TAG = "MCP23S17";
	Mutex mutex;
	std::shared_ptr<SPIDevice> spidev;
	gpio_num_t irqPin = GPIO_NUM_NC;
	uint8_t devAddr = 0;
	
	void Transmit(uint8_t * txData, uint8_t * rxData, uint8_t count);

	uint8_t Read8(uint8_t reg);
	void Write8(uint8_t reg, uint8_t value);
	uint16_t Read16(uint8_t reg);
	void Write16(uint8_t reg, uint16_t value);
	
	Pins pinBuffer = Pins::NONE;
	Pins pinDirBuffer = Pins::ALL;

public:
		
	struct Config
	{
		gpio_num_t IRQPin;
	};
	
	MCP23S17(std::shared_ptr<SPIDevice> spidevice);
	void SetPinsMode(Pins mask, PinModes mode);
	void SetPins(Pins mask, Pins value);
	Pins GetPins(Pins mask);
	void ConsecutivePinWriting(Pins mask, Pins* values, size_t size);
	void setConfig(const Config& newConfig);
    void init();
    bool isInitialized() const;

	private:
    	Config config_;
    	bool initialized_ = false;
};

DEFINE_ENUM_CLASS_FLAG_OPERATORS(MCP23S17::Pins, uint32_t);

