#pragma once
#include "mcp23s17.h"

namespace ESP_Drivers
{
	
	typedef enum
	{
		T6963C_BACKLIGHT_OFF,
		T6963C_BACKLIGHT_MIN,
		T6963C_BACKLIGHT_MID,
		T6963C_BACKLIGHT_MAX,
	}t6963c_backlight_t;


	class T6963C
	{
		MCP23S17* expander;
		uint8_t rows = 0;
		uint8_t columns = 0;
		void WriteByte(bool cd, uint8_t data);
		void SetCtrlPins(bool wr, bool rd, bool cd, bool ce);
		void SetDataPins(uint8_t data);
		void WriteCmd(uint8_t cmd);
		void WriteCmd(uint8_t cmd, uint8_t data1);
		void WriteCmd(uint8_t cmd, uint8_t data1, uint8_t data2);
		void Clear();
		void SetAddress(uint8_t col, uint8_t row);
		void SetCursor(uint8_t col, uint8_t row);
		void StartAutoWrite();
		void StopAutoWrite();
		void AutoWrite(uint8_t data);
		void AutoWriteChar(char data);
		void StatusCheck() {}	
		void OSet(uint8_t data, bool wr, bool rd, bool cd, bool ce, mcp23s17_pins_t* order);
		mcp23s17_pins_t* OWriteByte(bool cd, uint8_t data, mcp23s17_pins_t* order);
		mcp23s17_pins_t* OWriteCmd(uint8_t cmd, uint8_t data1, mcp23s17_pins_t* order);
		mcp23s17_pins_t* OWriteCmd(uint8_t cmd, uint8_t data1, uint8_t data2, mcp23s17_pins_t* order);
		mcp23s17_pins_t* OWriteCmd(uint8_t cmd, mcp23s17_pins_t* order);
		mcp23s17_pins_t* OSetAddress(uint8_t col, uint8_t row, mcp23s17_pins_t* order);
	
	public:
		struct Settings
		{
			uint16_t width = 128;
			uint16_t height = 128;
		};

		Settings settings;
		esp_err_t Init(MCP23S17* expander);
		void SetBacklight(t6963c_backlight_t value);
		void WriteRow(uint32_t y, uint8_t* data, size_t size);
	};
}