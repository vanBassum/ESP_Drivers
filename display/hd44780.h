#pragma once
#include "esp_system.h"
#include "mcp23s17.h"
#include <string>
//https://github.com/Matiasus/HD44780/blob/master/lib/hd44780.c

namespace ESP_Drivers
{
	class HD44780
	{
		MCP23S17* expander;

		void SetCursor(int x, int row);
		void LCD_cmd(unsigned char cmd);
		void WaitBFClear();
		void LCD_Data(unsigned char cmd);
		
	public:
		esp_err_t Init(MCP23S17* expander);
		void SetBacklight(bool enabled);
		void Write(std::string message);
		void Write(std::string message, int x, int y);
	};

}


