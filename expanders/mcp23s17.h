#pragma once
#include "kernel.h"
#include "esp_base.h"
#include "spi/bus.h"

//https://github.com/RobTillaart/MCP23S17/blob/master/MCP23S17.cpp
//https://www.esp32.com/viewtopic.php?t=9309

class MCP23S17
{
public:
	enum class Pins
	{
		NONE = 0,
		A0   = (1 << 0),
		A1   = (1 << 1),
		A2   = (1 << 2),
		A3   = (1 << 3),
		A4   = (1 << 4),
		A5   = (1 << 5),
		A6   = (1 << 6),
		A7   = (1 << 7),
		B0   = (1 << 8),
		B1   = (1 << 9),
		B2   = (1 << 10),
		B3   = (1 << 11),
		B4   = (1 << 12),
		B5   = (1 << 13),
		B6   = (1 << 14),
		B7   = (1 << 15),
		ALL  = 0xFFFF,
	};
	
	enum class PinModes
	{
		INPUT,
		OUTPUT,
	};
		
private:
	const char* TAG = "MCP23S17";
	Mutex mutex;
	SPIDevice& spidev;
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
		
	MCP23S17(SPIDevice& spiDev, gpio_num_t irq);
	void SetPinsMode(Pins mask, PinModes mode);
	void SetPins(Pins mask, Pins value);
	Pins GetPins(Pins mask);
	void ConsecutivePinWriting(Pins mask, Pins* values, size_t size);
	
};

DEFINE_ENUM_CLASS_FLAG_OPERATORS(MCP23S17::Pins, uint32_t);

