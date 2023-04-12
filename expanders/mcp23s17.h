#pragma once
#include "kernel.h"
#include "esp_base.h"
#include "spi/bus.h"

//https://github.com/RobTillaart/MCP23S17/blob/master/MCP23S17.cpp
//https://www.esp32.com/viewtopic.php?t=9309

	enum mcp23s17_pins_t
	{
		MCP23S17_PIN_NONE = 0,
		MCP23S17_PIN_A0   = (1 << 0),
		MCP23S17_PIN_A1   = (1 << 1),
		MCP23S17_PIN_A2   = (1 << 2),
		MCP23S17_PIN_A3   = (1 << 3),
		MCP23S17_PIN_A4   = (1 << 4),
		MCP23S17_PIN_A5   = (1 << 5),
		MCP23S17_PIN_A6   = (1 << 6),
		MCP23S17_PIN_A7   = (1 << 7),
		MCP23S17_PIN_B0   = (1 << 8),
		MCP23S17_PIN_B1   = (1 << 9),
		MCP23S17_PIN_B2   = (1 << 10),
		MCP23S17_PIN_B3   = (1 << 11),
		MCP23S17_PIN_B4   = (1 << 12),
		MCP23S17_PIN_B5   = (1 << 13),
		MCP23S17_PIN_B6   = (1 << 14),
		MCP23S17_PIN_B7   = (1 << 15),
		MCP23S17_PIN_ALL  = 0xFFFF,
	};
	DEFINE_ENUM_FLAG_OPERATORS(mcp23s17_pins_t)
	
	enum mcp23s17_pinmodes_t
	{
		MCP23S17_PINMODE_INPUT,
		MCP23S17_PINMODE_OUTPUT
	}
	;


	class MCP23S17
	{
		Mutex mutex;
		SPI::Device spidev;
		gpio_num_t irqPin = GPIO_NUM_NC;
		uint8_t devAddr = 0;
	
		esp_err_t Transmit(uint8_t * txData, uint8_t * rxData, uint8_t count);

		uint8_t Read8(uint8_t reg);
		void Write8(uint8_t reg, uint8_t value);
		uint16_t Read16(uint8_t reg);
		void Write16(uint8_t reg, uint16_t value);
	
		uint16_t pinBuffer = 0;
		uint16_t pinDirBuffer = 0xFFFF;

	public:
		bool Init(SPI::Bus* spiBus, gpio_num_t cs, gpio_num_t irq, transaction_cb_t pre_cb, transaction_cb_t post_cb);
		void SetPinsMode(mcp23s17_pins_t mask, mcp23s17_pinmodes_t mode);
		void SetPins(mcp23s17_pins_t mask, mcp23s17_pins_t value);
		mcp23s17_pins_t GetPins(mcp23s17_pins_t mask);
		void ConsecutivePinWriting(mcp23s17_pins_t mask, mcp23s17_pins_t* values, size_t size);
	
	};